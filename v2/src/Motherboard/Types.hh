#ifndef TYPES_HH_
#define TYPES_HH_

typedef int32_t micros_t;

#define AXIS_COUNT 3

class Point {
private:
	int32_t coordinates[AXIS_COUNT];
public:
	Point() {}
	Point(int32_t x, int32_t y, int32_t z) {
		coordinates[0] = x;
		coordinates[1] = y;
		coordinates[2] = z;
	}
	const int32_t& operator[](unsigned int index) const { return coordinates[index]; }
} __attribute__ ((__packed__));


#endif /* TYPES_HH_ */
