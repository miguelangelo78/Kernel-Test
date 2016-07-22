/*
 * time.h
 *
 *  Created on: 30/03/2016
 *      Author: Miguel
 */

#ifndef SRC_TIME_H_
#define SRC_TIME_H_

#include <stdint.h>
#ifdef __cplusplus
#include <module.h>
#endif

typedef struct {
	uint8_t sec, min, hour;
	uint8_t weekday, monthday;
	uint8_t month;
	uint16_t year;
} time_t;

static inline uint32_t time_secs_of_years(int years) {
	uint32_t days = 0;
	years += 2000;
	while (years > 1969) {
		days += 365;
		if (years % 4 == 0) {
			if (years % 100 == 0) {
				if (years % 400 == 0) {
					days++;
				}
			} else {
				days++;
			}
		}
		years--;
	}
	return days * 86400;
}

static inline uint32_t time_secs_of_month(int months, int year) {
	year += 2000;

	uint32_t days = 0;
	switch(months) {
		case 11:
			days += 30;
		case 10:
			days += 31;
		case 9:
			days += 30;
		case 8:
			days += 31;
		case 7:
			days += 31;
		case 6:
			days += 30;
		case 5:
			days += 31;
		case 4:
			days += 30;
		case 3:
			days += 31;
		case 2:
			days += 28;
			if ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)))
				days++;
		case 1:
			days += 31;
		default:
			break;
	}
	return days * 86400;
}

inline uint32_t now() {
	uint32_t _now;
	MOD_IOCTLD("cmos_driver", _now, 4);
	return _now;
}

#endif /* SRC_TIME_H_ */
