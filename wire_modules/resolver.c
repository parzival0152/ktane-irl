enum wire_color {
	NO_COLOR = 100,
	WHITE = 1400,
	BLUE = 2100,
	YELLOW = 2500,
	RED = 3500,
	BLACK = 4080
};

uint8_t count_color(enum wire_color* wire_list, uint8_t length, enum wire_color color) {
	uint8_t count = 0;
	for(int i = 0; i < length; i++) {
		count += wire_list[i] == color;
	}
	return count;
}

uint8_t last_of_color(enum wire_color* wire_list, uint8_t length, enum wire_color color) {
	uint8_t index = 0;
	for(int i = 0; i < length; i++) {
		index = wire_list[i] == color ? i : index;
	}
	return index;
}

uint8_t wire_3(enum wire_color* wire_list, bool last_serial_odd) {
	uint8_t red_count = count_color(wire_list, 3, RED);
	uint8_t blue_count = count_color(wire_list, 3, BLUE);

	if(red_count == 0) return 1;
	if(wire_list[2] == WHITE) return 2;
	if(blue_count > 1) return last_of_color(wire_list, 3, BLUE);
	return 2;
}

uint8_t wire_4(enum wire_color* wire_list, bool last_serial_odd) {
	uint8_t red_count = count_color(wire_list, 4, RED);
	uint8_t blue_count = count_color(wire_list, 4, BLUE);
	uint8_t yellow_count = count_color(wire_list, 4, YELLOW);
	
	if(red_count > 1 && last_serial_odd) return last_of_color(wire_list, 3, RED);
	if(wire_list[3] == YELLOW && red_count == 0) return 0;
	if(blue_count == 1) return 0;
	if(yellow_count > 1) return 3;
	return 1;
}

uint8_t wire_5(enum wire_color* wire_list, bool last_serial_odd) {
	uint8_t red_count = count_color(wire_list, 5, RED);
	uint8_t black_count = count_color(wire_list, 5, BLACK);
	uint8_t yellow_count = count_color(wire_list, 5, YELLOW);
	
	if(wire_list[4] == BLACK && last_serial_odd) return 3;
	if(red_count == 1 && yellow_count > 1) return 0;
	if(black_count == 0) return 1;
	return 0;
}

uint8_t wire_6(enum wire_color* wire_list, bool last_serial_odd) {
	uint8_t red_count = count_color(wire_list, 6, RED);
	uint8_t yellow_count = count_color(wire_list, 6, YELLOW);
	uint8_t white_count = count_color(wire_list, 6, WHITE);
	
	if(yellow_count == 0 && last_serial_odd) return 2;
	if(yellow_count == 1 && white_count > 1) return 3;
	if(red_count == 0) return 5;
	return 3;
}


uint8_t get_correct_index(enum wire_color* wire_list, uint8_t length, bool last_serial_odd) {
	switch(length) {
		case 3:
			return wire_3(wire_list, last_serial_odd);
			break;
		case 4:
			return wire_4(wire_list, last_serial_odd);
			break;
		case 5:
			return wire_5(wire_list, last_serial_odd);
			break;
		case 6:
			return wire_6(wire_list, last_serial_odd);
			break;
		default:
			printf("ERROR: Length is not in the defined range of the Module");
			return 0;
			break;
	}
}
