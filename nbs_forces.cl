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

__kernel void execute(__global struct body_t* bodies, float G, int n){
	int i = get_global_id(0);

    if(i < n){
    struct body_t a = bodies[i];
    float EPS = 3e4;

    float fx = 0;
    float fy = 0;
    float fz = 0;

    // Calculate force
    for(int idx = 0; idx < n; idx++){
        if(idx != i){
            struct body_t b = bodies[idx];

            float dx = b.px - a.px;
            float dy = b.py - a.py;
            float dz = b.pz - a.pz;
            float dist = sqrt(dx*dx + dy*dy + dz*dz);

            float F = (G * a.mass * b.mass) / (dist * dist + EPS * EPS);

            fx += F * dx / dist;
            fy += F * dy / dist;
            fz += F * dz / dist;
        }
    }

    a.fx = fx;
    a.fy = fy;
    a.fz = fz;

    bodies[i] = a;
    }
}

