
const char words[16][6] = {
	"shell",
	"halls",
	"slick",
	"trick",
	"boxes",
	"leaks",
	"strobe",
	"bistro",
	"flick",
	"bombs",
	"break",
	"brick",
	"steak",
	"sting",
	"vector",
	"beats"
};

const char freqs[16][4] = {
	"505\0",
	"515\0",
	"522\0",
	"532\0",
	"535\0",
	"542\0",
	"545\0",
	"552\0",
	"555\0",
	"565\0",
	"572\0",
	"575\0",
	"582\0",
	"592\0",
	"595\0",
	"600\0",
};

void print_freq(uint8_t index){
	printf("The current freq is: 3.");
	switch(index){
		case 0:
			printf("505");
			break;
		case 1:
			printf("515");
			break;
		case 2:
			printf("522");
			break;
		case 3:
			printf("532");
			break;
		case 4:
			printf("535");
			break;
		case 5:
			printf("542");
			break;
		case 6:
			printf("545");
			break;
		case 7:
			printf("552");
			break;
		case 8:
			printf("555");
			break;
		case 9:
			printf("565");
			break;
		case 10:
			printf("572");
			break;
		case 11:
			printf("575");
			break;
		case 12:
			printf("582");
			break;
		case 13:
			printf("592");
			break;
		case 14:
			printf("595");
			break;
		case 15:
			printf("600");
			break;
		default:
			break;
	}
	printf(" MHz\n");
}
