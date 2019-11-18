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

__kernel void execute(__global struct body_t* bodies, const float dt, const int n){
	int i = get_global_id(0);

    if(i < n){
		struct body_t a = bodies[i];
		a.vx += dt * a.fx / a.mass;
		a.vy += dt * a.fy / a.mass;
		a.vz += dt * a.vz / a.mass;

		a.px += dt * a.vx;
		a.py += dt * a.vy;
		a.pz += dt * a.vz;

		bodies[i] = a;
    }
}

