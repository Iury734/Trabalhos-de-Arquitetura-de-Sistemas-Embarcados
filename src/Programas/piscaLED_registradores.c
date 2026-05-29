int main(void) {
    *((volatile unsigned int *)0x40048038) |= 0x400;
    *((volatile unsigned int *)0x4004A04C) = 0x100;
    *((volatile unsigned int *)0x400FF054) |= (1 << 19);

    while (1) {
        *((volatile unsigned int *)0x400FF04C) = (1 << 19);
        for (volatile int i = 0; i < 2500000; i++);
    }
}