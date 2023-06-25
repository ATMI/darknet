// #ifdef OPENCV

#include "stdio.h"
#include "stdlib.h"
#include "opencv2/opencv.hpp"
#include "image.h"

using namespace cv;

extern "C" {

/*
IplImage *image_to_ipl(image im)
{
    int x,y,c;
    IplImage *disp = cvCreateImage(cvSize(im.w,im.h), IPL_DEPTH_8U, im.c);
    int step = disp->widthStep;
    for(y = 0; y < im.h; ++y){
        for(x = 0; x < im.w; ++x){
            for(c= 0; c < im.c; ++c){
                float val = im.data[c*im.h*im.w + y*im.w + x];
                disp->imageData[y*step + x*im.c + c] = (unsigned char)(val*255);
            }
        }
    }
    return disp;
}

image ipl_to_image(IplImage* src)
{
    int h = src->height;
    int w = src->width;
    int c = src->nChannels;
    image im = make_image(w, h, c);
    unsigned char *data = (unsigned char *)src->imageData;
    int step = src->widthStep;
    int i, j, k;

    for(i = 0; i < h; ++i){
        for(k= 0; k < c; ++k){
            for(j = 0; j < w; ++j){
                im.data[k*w*h + i*w + j] = data[i*step + j*c + k]/255.;
            }
        }
    }
    return im;
}
*/

Mat image_to_mat(image im) {
	image copy = copy_image(im);
	constrain_image(copy);
	if (im.c == 3) rgbgr_image(copy);

	cv::Mat m(im.w, im.h, CV_8UC(im.c));

	int step = m.step;
	int x, y, c;

	for (y = 0; y < im.h; ++y) {
		for (x = 0; x < im.w; ++x) {
			for (c = 0; c < im.c; ++c) {
				float val = im.data[c * im.h * im.w + y * im.w + x];
				m.data[y * step + x * im.c + c] = (unsigned char) (val * 255);
			}
		}
	}

	free_image(copy);
	return m;
}

image mat_to_image(const Mat &src) {
	/*IplImage ipl = m;
	image im = ipl_to_image(&ipl);*/

	int h = src.rows;
	int w = src.cols;
	int c = src.channels();
	image im = make_image(w, h, c);
	unsigned char *data = (unsigned char *) src.data;
	int step = src.step;
	int i, j, k;

	for (i = 0; i < h; ++i) {
		for (k = 0; k < c; ++k) {
			for (j = 0; j < w; ++j) {
				im.data[k * w * h + i * w + j] = data[i * step + j * c + k] / 255.;
			}
		}
	}

	rgbgr_image(im);
	return im;
}

void *open_video_stream(const char *f, int c, int w, int h, int fps) {
	VideoCapture *cap;
	if (f) cap = new VideoCapture(f);
	else cap = new VideoCapture(c);
	if (!cap->isOpened()) return 0;
	if (w) cap->set(CAP_PROP_FRAME_WIDTH, w);
	if (h) cap->set(CAP_PROP_FRAME_HEIGHT, w);
	if (fps) cap->set(CAP_PROP_FPS, w);
	return (void *) cap;
}

image get_image_from_stream(void *p) {
	VideoCapture *cap = (VideoCapture *) p;
	Mat m;
	*cap >> m;
	if (m.empty()) return make_empty_image(0, 0, 0);
	return mat_to_image(m);
}

image load_image_cv(char *filename, int channels) {
	int flag = -1;
	if (channels == 0) flag = -1;
	else if (channels == 1) flag = 0;
	else if (channels == 3) flag = 1;
	else {
		fprintf(stderr, "OpenCV can't force load with %d channels\n", channels);
	}
	Mat m;
	m = imread(filename, flag);
	if (!m.data) {
		fprintf(stderr, "Cannot load image \"%s\"\n", filename);
		char buff[256];
		sprintf(buff, "echo %s >> bad.list", filename);
		system(buff);
		return make_image(10, 10, 3);
		//exit(0);
	}

	return mat_to_image(m);
}

image resize_image_cv(image im, int w, int h) {
	printf("Using OpenCV resize\n");
	image result = make_image(w, h, im.c);

	for (int i = 0; i < im.c; ++i) {
		cv::Mat original(im.h, im.w, CV_32FC1, &im.data[im.w * im.h * i]);
		cv::Mat resized(h, w, CV_32FC1, &result.data[w * h * i]);
		cv::resize(original, resized, cv::Size(w, h), 0, 0, INTER_AREA);
	}

	return result;
}


void place_image_cv(image im, int w, int h, int dx, int dy, image canvas) {
	// printf("Using OpenCV place\n");
	// Scale, then move
	cv::Mat mat = (Mat_<float>(2, 3) << float(w) / float(im.w), 0, dx, 0, float(h) / float(im.h), dy);

	for (int i = 0; i < im.c; ++i) {
		cv::Mat result(canvas.h, canvas.w, CV_32FC1, &canvas.data[canvas.w * canvas.h * i]);
		cv::Mat original(im.h, im.w, CV_32FC1, &im.data[im.w * im.h * i]);
		cv::warpAffine(original, result, mat, cv::Size(result.cols, result.rows), INTER_AREA);
	}
}

int show_image_cv(image im, const char *name, int ms) {
	Mat m = image_to_mat(im);
	imshow(name, m);
	int c = waitKey(ms);
	if (c != -1) c = c % 256;
	return c;
}

void make_window(char *name, int w, int h, int fullscreen) {
	namedWindow(name, WINDOW_NORMAL);
	if (fullscreen) {
		setWindowProperty(name, WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
	}
	else {
		resizeWindow(name, w, h);
		if (strcmp(name, "Demo") == 0) moveWindow(name, 0, 0);
	}
}

}

// #endif
