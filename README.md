# Radiosity

The aim of this project is to construct a radiosity-based
diffuse-diffuse interaction renderer based on half-remembered
fifteen-plus-year-old knowledge and working out again from first
principles.

## Reconstructing the theory

We'll start with a matt wall, lit evenly, looks a single colour. It
doesn't get brighter or darker depending on the angle we view it at,
or the distance we view it from. Put another way, the amount of light
our eye receives just depends on how much light the wall emits per
area, and the angle it subtends in our vision.

What does this mean physically?

 * In terms of scale independence, the light will fall off with the
   square of the distance, but so will the area, so the light per area
   remains constant.

 * If light-received-per-area-seen-by-viewer remains constant, the
   amount of light emitted given off must drop off with angle, as the
   amount of surface-per-area-facing-the-viewer increases. This factor
   is cosine of the angle between the viewer and the normal.

We can thus write the amount of light received at a point as

`L = B * A * cos(theta)`

Where `L` is the amount of light, `B` is the brightness of the
emitting surface, `A` is its area, and `theta` is the angle between
the normal of the emitting surface and the ray to the receiver.

How do we find the amount of light received by an area?

Assuming the light comes from a reasonably-point-like light source,
the amount of light received will be proportional to the area, as
measured perpendicular to the ray from the emitter. Putting it all
together, the equation is:

`L_er = B_e * A_e * cos(theta_e) * A_r * cos(theta_r)`

where `e` is the value for the emitter, `r` is the value for the
receiver, and the parameters are otherwise the same as previously.

Rather neatly, the equation is symmetric, demonstrating the nice
symmetry between forwards and backwards ray-tracing, and how the
radiosity solution represents an equilibrium where the net light flow
between pairs of surfaces is zero.
