#include "hwSobel.h"

/*void sobel_accel(xf::Mat<IM_TYPE, HEIGHT, WIDTH, NPC> &src,
		xf::Mat<IM_TYPE, HEIGHT, WIDTH, NPC> &gx,
		xf::Mat<IM_TYPE, HEIGHT, WIDTH, NPC> &gy,
		xf::Mat<IM_TYPE, HEIGHT, WIDTH, NPC> &mag)
{
	xf::Sobel<XF_BORDER_CONSTANT, FILTER_WIDTH, IM_TYPE, GRAD_TYPE, HEIGHT, WIDTH, NPC>(imgInput, gx, gy);

	xf::magnitude<MAG_TYPE, GRAD_TYPE, IM_TYPE, HEIGHT, WIDTH, NPC>(gx, gy, mag);

}
*/

void sobel_accel(xf::Mat<IM_TYPE, HEIGHT, WIDTH, NPC> &src,
		xf::Mat<IM_TYPE, HEIGHT, WIDTH, NPC> &gx,
		xf::Mat<IM_TYPE, HEIGHT, WIDTH, NPC> &gy)
{
	xf::Sobel<XF_BORDER_CONSTANT, FILTER_WIDTH, IM_TYPE, IM_TYPE, HEIGHT, WIDTH, NPC>(src, gx, gy);
}
