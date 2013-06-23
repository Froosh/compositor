// This part is specific to the procedural animation.

// Here's where we set it up
void setup() {
	return;
}

// And we try to call this fairly often - every 20 milliseconds or thereabouts
//
void loop() {
	int pixnum, r, g, b, choix, choix2;
	uint8_t max_range = 0x7F;
	
	for (pixnum =0; pixnum < NUM_GLOBES; pixnum++) {
		r = rand() & max_range;
		g = rand() & max_range;
		b = rand() & max_range;
		choix = rand() & 0x03;
		switch (choix) {
			case 0x00:
			{
				r = 0x00;
				break;
			}
			case 0x01:
			{
				g = 0x00;
				break;
			}
			case 0x02:
			{
				b = 0x00;
				break;
			}
			case 0x03:
			{
				choix2 = rand() & 0x03;
				switch(choix2) {
					case 0:
						r = g = 0;
						break;
					case 1:
						r = b = 0;
						break;
					case 2:
						b = g = 0;
						break;
					case 3:
						break;
				}

			}
		}
		set_pixel(pixnum, r, g, b);
	}
	return;
}
