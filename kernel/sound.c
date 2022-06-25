#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "sound.h"
#define PCIE_ECAM 0x30000000L
#define PCIE_PIO 0x3000000L
#define FOR(i, a, b) for (uint32 i = (a), i##_END_ = (b); i <= i##_END_; ++i)

// all registers address can be found in https://wiki.osdev.org/AC97

// Reference to Intel doc AC97

static struct spinlock soundLock;
static struct soundNode *soundQueue;

struct descriptor {
    uint buf;
    uint cmd_len;
};

static struct descriptor descriTable[DMA_BUF_NUM];

ushort namba; // native audio mixer base address
ushort nabmba; // native audio bus mastering base address

volatile uchar* RegByte(uint64 reg) {return (volatile uchar *)(reg);}
volatile uint32* RegInt(uint64 reg) {return (volatile uint32 *)(reg);}
volatile ushort* RegShort(uint64 reg) {return (volatile ushort *)(reg);}
uchar ReadRegByte(uint64 reg) {return *RegByte(PCIE_PIO | reg);}
void WriteRegByte(uint64 reg, uchar v) {*RegByte(PCIE_PIO | reg) = v;}
uint32 ReadRegInt(uint64 reg) {return *RegInt(PCIE_PIO | reg);}
void WriteRegInt(uint64 reg, uint32 v) {*RegInt(PCIE_PIO | reg) = v;}
ushort ReadRegShort(uint64 reg) {return *RegShort(PCIE_PIO | reg);}
void WriteRegShort(uint64 reg, ushort v) {*RegShort(PCIE_PIO | reg) = v;}

// Ecam: PCI Configuration Space
uint32 read_pci_config_int(uint32 bus, uint32 slot, uint32 func, uint32 offset) {
    return *RegInt((bus << 16) | (slot << 11) | (func << 8) | (offset) | PCIE_ECAM);
}

uint32 read_pci_config_byte(uint32 bus, uint32 slot, uint32 func, uint32 offset) {
    return *RegByte((bus << 16) | (slot << 11) | (func << 8)  | (offset) | PCIE_ECAM);
}

void write_pci_config_byte(uint32 bus, uint32 slot, uint32 func, uint32 offset, uchar val) {
    *RegByte((bus << 16) | (slot << 11) | (func << 8)  | (offset) | PCIE_ECAM) = val;
}

void write_pci_config_int(uint32 bus, uint32 slot, uint32 func, uint32 offset, uint32 val) {
    *RegInt((bus << 16) | (slot << 11) | (func << 8)   | (offset) | PCIE_ECAM) = val;
}

void write_pci_config_short(uint32 bus, uint32 slot, uint32 func, uint32 offset, ushort val) {
    *RegShort((bus << 16) | (slot << 11) | (func << 8)  | (offset) | PCIE_ECAM) = val;
}


void soundcard_init(uint32 bus, uint32 slot, uint32 func) {
    //Initailize Interruption
    initlock(&soundLock, "sound");

    // Initializing the Audio I/O Space
    write_pci_config_byte(bus, slot, func, 0x4, 0x5); // reg1: status|command, set Bus Master=1, IO space = 1
    // reg2: class=04, subclass=01, prog IF=00, revision ID=01

    //// get BAR0
    // Write Native Audio Mixer Base Address
    write_pci_config_int(bus, slot, func, 0x10, 0x0001); // reg4: BAR0
    namba = read_pci_config_int(bus, slot, func, 0x10) & (~0x1); // reg4: BAR0, IO space

    //// get BAR1
    // Write Native Audio Bus Mastering Base Address
    write_pci_config_int(bus, slot, func, 0x14, 0x0401); // reg5: BAR1
    nabmba = read_pci_config_int(bus, slot, func, 0x14) & (~0x1); // reg5: BAR1, IO space
    printf("AUDIO I/O Space initialized successfully!, namba=%x, nabmba=%x\n", namba, nabmba);

    // Hardware Interrupt Routing
    // dword: Byte
    // word: Short

    // Removing AC_RESET#
    WriteRegByte(nabmba + 0x2c, 0x2); //  Global Control Register: Cold Reset, no interrupt
    printf("AC_RESET removed successfully!\n");

    // Check until codec ready
    uint32 wait_time = 1000;
    while (!((ReadRegInt(nabmba + 0x30))) && wait_time) { // Global Status Register
        --wait_time;
    }
    if (!wait_time) {
        panic("Audio Init failed 0.");
        return;
    }
    printf("Codec is ready!, Global Control = %x\n", ReadRegInt(nabmba + 0x2c));
    printf("Codec is ready!, Global Status = %x\n", ReadRegInt(nabmba + 0x30));

    // Determining the Audio Codec
    uint tmp = ReadRegShort(namba + 0x2); // Set Master Output Volume
    WriteRegShort(namba + 0x2, 0x0808); // Volume = Mute
    if ((ReadRegShort(namba + 0x2)) != 0x0808) {
        panic("Audio Init failed 1.");
        return;
    }
    printf("Audio Codec Functionsd is found, current volume is %x.\n", tmp);

    // Reading the Audio Codec Vendor ID
    uint32 vendorID1 = ReadRegShort(namba + 0x7c);
    uint32 vendorID2 = ReadRegShort(namba + 0x7e);

    // Programming the PCI Audio Subsystem ID
    uint32 vendorID = (vendorID2 << 16) + vendorID1;
    write_pci_config_int(bus, slot, func, 0x2c, vendorID); // regB: Subsystem ID | Subsystem Vendor ID
    printf("Audio Codec Vendor ID read successfully!, vendorID is %x\n", vendorID);

    // Buffer Descriptor List
    uint base = v2p(descriTable);
    WriteRegInt(nabmba + 0x10, base); // NABM register box for PCM OUT
}

void soundinit(void) {
    // scan the pci configuration space
    FOR(bus, 0, 4)
        FOR(slot, 0, 31)
            FOR(func, 0, 7) {
                uint32 res = read_pci_config_int(bus, slot, func, 0x00); // reg0: device | vendor
                uint32 vendor = res & 0xffff;
                uint32 device = (res >> 16) & 0xffff;
                if (vendor == 0x8086 && device == 0x2415) {
                    printf("Find sound card! at bus=%d, slot=%d, func=%d\n", bus, slot, func);
                    soundcard_init(bus, slot, func);
                    return;
                }
            }
    printf("Sound card not found!\n");
}

void setSoundSampleRate(uint samplerate) {
    //Control Register --> 0x00
    //pause audio
    //disable interrupt
    WriteRegByte(nabmba + 0x1B, 0x00);
    //PCM Front DAC Rate
    WriteRegShort(namba + 0x2C, samplerate & 0xFFFF); // Sample rate of front speaker
    //PCM Surround DAC Rate
    WriteRegShort(namba + 0x2E, samplerate & 0xFFFF);
    //PCM LFE DAC Rate
    WriteRegShort(namba + 0x30, samplerate & 0xFFFF);
}

void soundInterrupt(void) {
    int i;
    acquire(&soundLock);

    struct soundNode *node = soundQueue;
    if(node == 0) {
        release(&soundLock);
        return;
    }
    soundQueue = node->next;

    //flag
    int flag = node->flag;

    node->flag |= PROCESSED;

    //0 sound file left
    WriteRegShort(nabmba + 0x16, 0x1c);
    if (soundQueue == 0) {
        release(&soundLock);
        return;
    }

    //descriptor table buffer
    for (i = 0; i < DMA_BUF_NUM; i++)
    {
        descriTable[i].buf = v2p(soundQueue->data) + i * DMA_BUF_SIZE;
        descriTable[i].cmd_len = 0x80000000 + DMA_SMP_NUM;
    }

    //play music

    WriteRegByte(nabmba + 0x1B, 0x05);

    release(&soundLock);
}

void playSound(void) {
    int i;

    //遍历声卡DMA的描述符列表，初始化每一个描述符buf指向缓冲队列中第一个音乐的数据块
    //每个数据块大小: DMA_BUF_SIZE
    for (i = 0; i < DMA_BUF_NUM; i++)
    {
        descriTable[i].buf = v2p(soundQueue->data) + i * DMA_BUF_SIZE;
        descriTable[i].cmd_len = 0x80000000 + DMA_SMP_NUM;
    }

    //开始播放: PCM_OUT
    if ((soundQueue->flag & PCM_OUT) == PCM_OUT)
    {
        //init base register
        //将内存地址base开始的1个双字写到PO_BDBAR
        //init last valid index
        WriteRegByte(nabmba + 0x15, 0x1F);
        //init control register
        //run audio
        //enable interrupt
        WriteRegByte(nabmba + 0x1B, 0x05);
        WriteRegShort(nabmba + 0x16, 0x1c);
    }
}

//add sound-piece to the end of queue
void addSound(struct soundNode *node) {
    acquire(&soundLock);

    struct soundNode **ptr;

    node->next = 0;
    for(ptr = &soundQueue; *ptr; ptr = &(*ptr)->next)
        ;
    *ptr = node;
    //node is already the first
    //play sound
    if (soundQueue == node)
    {
        playSound();
    }

    release(&soundLock);
}
