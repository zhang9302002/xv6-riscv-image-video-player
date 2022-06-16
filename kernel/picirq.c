// Intel 8259A programmable interrupt controllers.

#include "types.h"

#define PCIE_ECAM 0x30000000L
#define PCIE_PIO 0x3000000L

volatile uchar* RegByte(uint64 reg) {return (volatile uchar *)(reg);}
volatile uint32* RegInt(uint64 reg) {return (volatile uint32 *)(reg);}
volatile ushort* RegShort(uint64 reg) {return (volatile ushort *)(reg);}
uchar ReadRegByte(uint64 reg) {return *RegByte(PCIE_PIO | reg);}
void WriteRegByte(uint64 reg, uchar v) {*RegByte(PCIE_PIO | reg) = v;}
uint32 ReadRegInt(uint64 reg) {return *RegInt(PCIE_PIO | reg);}
void WriteRegInt(uint64 reg, uint32 v) {*RegInt(PCIE_PIO | reg) = v;}
ushort ReadRegShort(uint64 reg) {return *RegShort(PCIE_PIO | reg);}
void WriteRegShort(uint64 reg, ushort v) {*RegShort(PCIE_PIO | reg) = v;}

// I/O Addresses of the two programmable interrupt controllers
#define IO_PIC1         0x20    // Master (IRQs 0-7)
#define IO_PIC2         0xA0    // Slave (IRQs 8-15)

#define IRQ_SLAVE       2       // IRQ at which slave connects to master

// Current IRQ mask.
// Initial IRQ mask has interrupt 2 enabled (for slave 8259A).
static ushort irqmask = 0xFFFF & ~(1<<IRQ_SLAVE);

static void
picsetmask(ushort mask)
{
    irqmask = mask;
    WriteRegByte(IO_PIC1+1, mask);
    WriteRegByte(IO_PIC2+1, mask >> 8);
}

void
picenable(int irq)
{
  picsetmask(irqmask & ~(1<<irq));
}

// Initialize the 8259A interrupt controllers.
void
picinit(void)
{
  // mask all interrupts
    WriteRegByte(IO_PIC1+1, 0xFF);
    WriteRegByte(IO_PIC2+1, 0xFF);

  // Set up master (8259A-1)

  // ICW1:  0001g0hi
  //    g:  0 = edge triggering, 1 = level triggering
  //    h:  0 = cascaded PICs, 1 = master only
  //    i:  0 = no ICW4, 1 = ICW4 required
    WriteRegByte(IO_PIC1, 0x11);

  // ICW2:  Vector offset
    WriteRegByte(IO_PIC1+1, 32);

  // ICW3:  (master PIC) bit mask of IR lines connected to slaves
  //        (slave PIC) 3-bit # of slave's connection to master
    WriteRegByte(IO_PIC1+1, 1<<IRQ_SLAVE);

  // ICW4:  000nbmap
  //    n:  1 = special fully nested mode
  //    b:  1 = buffered mode
  //    m:  0 = slave PIC, 1 = master PIC
  //      (ignored when b is 0, as the master/slave role
  //      can be hardwired).
  //    a:  1 = Automatic EOI mode
  //    p:  0 = MCS-80/85 mode, 1 = intel x86 mode
    WriteRegByte(IO_PIC1+1, 0x3);

  // Set up slave (8259A-2)
    WriteRegByte(IO_PIC2, 0x11);                  // ICW1
    WriteRegByte(IO_PIC2+1, 32 + 8);      // ICW2
    WriteRegByte(IO_PIC2+1, IRQ_SLAVE);           // ICW3
  // NB Automatic EOI mode doesn't tend to work on the slave.
  // Linux source code says it's "to be investigated".
    WriteRegByte(IO_PIC2+1, 0x3);                 // ICW4

  // OCW3:  0ef01prs
  //   ef:  0x = NOP, 10 = clear specific mask, 11 = set specific mask
  //    p:  0 = no polling, 1 = polling mode
  //   rs:  0x = NOP, 10 = read IRR, 11 = read ISR
    WriteRegByte(IO_PIC1, 0x68);             // clear specific mask
    WriteRegByte(IO_PIC1, 0x0a);             // read IRR by default

    WriteRegByte(IO_PIC2, 0x68);             // OCW3
    WriteRegByte(IO_PIC2, 0x0a);             // OCW3

    if(irqmask != 0xFFFF)
        picsetmask(irqmask);
}
