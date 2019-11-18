/*
 * body.h
 *
 *  Created on: Nov 18, 2019
 *      Author: bradly
 */

#ifndef BODY_H_
#define BODY_H_

typedef struct body_t {
	float px;
	float py;
	float pz;

	float vx;
	float vy;
	float vz;

	float fx;
	float fy;
	float fz;

	float mass;
};



#endif /* BODY_H_ */
