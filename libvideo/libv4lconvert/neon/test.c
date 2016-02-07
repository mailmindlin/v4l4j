void convert1() {
	
}
void convert8(unsigned long x) {
	
}
void testSwitch(unsigned int w, unsigned int h) {
	unsigned long int l = w * h;
	unsigned char i;
	for (unsigned char i = l % 4; i > 0; i--)
		convert1();
	l = l >> 2;
	convert8(l);
}


/*void f_1_370705() {
	float f1 = 1.370705;
}
void f_0_698001() {
	float f2 = 0.698001;
}
void f_0_337633() {
	float f3 = 0.337633;
}
void f_1_732446() {
	float f4 = 1.732446;
}*/