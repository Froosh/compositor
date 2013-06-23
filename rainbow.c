// This part is specific to the procedural animation.
#define MAX_TIMER 5		// Slow things down a wee bit
#define SHIFT_SIZE 12		// Make the maths more accurate

// Now -- fun with data structures
struct pixel { 
	unsigned char r; 
	unsigned char g; 
	unsigned char b; 
};
struct globes { 
	struct pixel values[NUM_GLOBES]; 
};
struct globes hol;

struct pixel initials[NUM_GLOBES] = {
	{ 0xFF, 0x00, 0x00 },	// 0
	{ 0xEF, 0x0F, 0x00 },	// 1
	{ 0xDF, 0x1F, 0x00 },
	{ 0xCF, 0x2F, 0x00 },
	{ 0xBF, 0x3F, 0x00 },	
	{ 0xAF, 0x4F, 0x00 },
	{ 0x9F, 0x5F, 0x00 },
	{ 0x8F, 0x6F, 0x00 },
	{ 0x7F, 0x7F, 0x00 },
	{ 0x6F, 0x8F, 0x00 },	
	{ 0x5F, 0x9F, 0x00 },
	{ 0x4F, 0xAF, 0x00 },
	{ 0x3F, 0xBF, 0x00 },
	{ 0x2F, 0xCF, 0x00 },
	{ 0x1F, 0xDF, 0x00 },	
	{ 0x0F, 0xEF, 0x00 },
	{ 0x00, 0xFF, 0x00 },	// 16
	{ 0x00, 0xF0, 0x0F },
	{ 0x00, 0xE1, 0x1E },
	{ 0x00, 0xD2, 0x2D },	
	{ 0x00, 0xC3, 0x3C },
	{ 0x00, 0xB4, 0x4B },
	{ 0x00, 0xA5, 0x5A },
	{ 0x00, 0x96, 0x69 },
	{ 0x00, 0x87, 0x78 },	
	{ 0x00, 0x78, 0x87 },
	{ 0x00, 0x69, 0x96 },
	{ 0x00, 0x5A, 0xA5 },
	{ 0x00, 0x4B, 0xB4 },
	{ 0x00, 0x3C, 0xC3 },	
	{ 0x00, 0x2D, 0xD2 },
	{ 0x00, 0x1E, 0xE1 },
	{ 0x00, 0x0F, 0xF0 },
	{ 0x00, 0x00, 0xFF },  // 33
	{ 0x0F, 0x00, 0xF0 },	
	{ 0x1E, 0x00, 0xE1 },
	{ 0x2D, 0x00, 0xD2 },
	{ 0x3C, 0x00, 0xC3 },
	{ 0x4B, 0x00, 0xB4 },
	{ 0x5A, 0x00, 0xA5 },	
	{ 0x69, 0x00, 0x96 },
	{ 0x78, 0x00, 0x87 },
	{ 0x87, 0x00, 0x78 },
	{ 0x96, 0x00, 0x69 },
	{ 0xA5, 0x00, 0x5A },	
	{ 0xB4, 0x00, 0x4B },
	{ 0xC3, 0x00, 0x3C },
	{ 0xD2, 0x00, 0x2D },
	{ 0xE1, 0x00, 0x1F },
	{ 0xF0, 0x00, 0x0F }	
};

int timer;

struct pixel interpolate(unsigned char begr, 
							unsigned char begg, 
							unsigned char begb, 
							unsigned char endr, 
							unsigned char endg, 
							unsigned char endb, 
							int max, int count) {

unsigned long wb[3], we[3], wr[3];
int k;
unsigned long diff, delta, fixup;
struct pixel retval;

	// In order to get some good arithmetics, we promote everything to int and shift left 7 bits
	wb[0] = begr << SHIFT_SIZE;
	wb[1] = begg << SHIFT_SIZE;
	wb[2] = begb << SHIFT_SIZE;
	we[0] = endr << SHIFT_SIZE;
	we[1] = endg << SHIFT_SIZE;
	we[2] = endb << SHIFT_SIZE;

	for (k = 0; k < 3; k++) {
		if (wb[k] == we[k]) {
			wr[k] = wb[k];		// If equal, pass through with no change
		} else {
			if (wb[k] > we[k]) {

				// Do this if we are decrementing
				diff = wb[k] - we[k];
				delta = diff / max;		// Get units per diff
				delta = delta * count;	// And now the total delta
				fixup = wb[k] - delta;	// This should be our interpolated value
				fixup = fixup >> SHIFT_SIZE;		// Now reducify it appropriately
				wr[k] = fixup;
				//printf("-");
			} else {
				// We are incrementing
				diff = we[k] - wb[k];
				delta = diff / max;		// Get units per diff
				delta = delta * count;	// And now the total delta
				fixup = wb[k] + delta;	// This should be our interpolated value
				fixup = fixup >> SHIFT_SIZE;		// Now reducify it appropriately
				wr[k] = fixup;
				//printf("+");
			}
		}
	}
	retval.r = wr[0];
	retval.g = wr[1];
	retval.b = wr[2];
	return retval;
}


// Here's where we set it up
void setup() {
int i;

	for (i=0; i < NUM_GLOBES; i++) {
		hol.values[i].r = initials[i].r;
		hol.values[i].g = initials[i].g;
		hol.values[i].b = initials[i].b;
	}
	timer = 0;
	return;
}

// And we try to call this fairly often - every 20 milliseconds or thereabouts
//
void loop() {
int i, j;
struct pixel saver;
struct pixel interp;

	// Interpolate until we move forward.
	if (++timer < MAX_TIMER) {
		for (j = 1; j < (NUM_GLOBES -2); j++) {
			interp = interpolate(hol.values[j].r,hol.values[j].g, hol.values[j].b,
				hol.values[j-1].r, hol.values[j-1].g, hol.values[j-1].b, MAX_TIMER, timer);
			set_pixel(j, interp.r, interp.g, interp.b);
			/*if (j == 20) {
				printf("Int 20 => %2x %2x %2x\n", interp.r, interp.g, interp.b);
			}*/
		}
		// Now do the wrap from the end of the string to the start
		interp = interpolate(hol.values[NUM_GLOBES-1].r,hol.values[NUM_GLOBES-1].g, hol.values[NUM_GLOBES-1].b,
				hol.values[0].r, hol.values[0].g, hol.values[0].b, MAX_TIMER, timer);
		set_pixel(NUM_GLOBES-1, interp.r, interp.g, interp.b);


	} else {

		timer = 0;


		// Rotate the values in the string forward.
		saver.r = hol.values[NUM_GLOBES-1].r;		// Save the high value
		saver.g = hol.values[NUM_GLOBES-1].g;		// Save the high value
		saver.b = hol.values[NUM_GLOBES-1].b;		// Save the high value

		// And copy it forward
		for (j = (NUM_GLOBES-1); j > 0; j--) {
			hol.values[j].r = hol.values[j-1].r;
			hol.values[j].g = hol.values[j-1].g;
			hol.values[j].b = hol.values[j-1].b;
		}
		hol.values[0].r = saver.r;	// And loop the string
		hol.values[0].g = saver.g;	// And loop the string
		hol.values[0].b = saver.b;	// And loop the string

		// Take the structure and write it out.
		for (i = 0; i < NUM_GLOBES; i++) {
			set_pixel(i, hol.values[i].r, hol.values[i].g, hol.values[i].b);
		}
		/*printf("NEW 20 => %2x %2x %2x\n", hol.values[20].r, hol.values[20].g, hol.values[20].b);
		printf("NEW 19 => %2x %2x %2x\n", hol.values[19].r, hol.values[19].g, hol.values[19].b);*/
	}
	return;
}
