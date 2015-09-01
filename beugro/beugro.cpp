//=============================================================================================
// Nemhivatalos szamitogepes grafika hazi gyakorlo keret 2015, beugro.
// Az Email definiciot ertelemszeruen toltsd ki.
// Tilos:
// - kozizlest serteni
// - rosszindulatu kodot hasznalni (adatgyujtes/tovabbitas, fajlok megvaltoztatasa, a tobbi program befolyasolasa, DOS-olas, stb.)
// ---------------------------------------------------------------------------------------------
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
#define Email   "gigu92@gmail.com"
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot a sajat szorakoztatasomra, kepemre es hasonlatossagomra keszitettem.
// Tisztaban vagyok azzal, hogy ami egyszer felkerult az internetre, azt onnan
// szinte lehetetlen eltuntetni, igy ha jelen kodomat nyilvanos helyen kozzeteszem,
// az adott esetben plagiumgyanut kelthet a hivatalos hazi feladat ertekelesekor.
//=============================================================================================

#if defined(__linux__)
#include <sys/time.h>
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#endif

#include <string.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2\calib3d\calib3d.hpp>

using namespace cv;
using namespace std;

long msec()
{
	static long fakeTime = 0;
	fakeTime += 40;
#if defined(__linux__)
	struct timeval tv;       
	if(gettimeofday(&tv, NULL) != 0) return fakeTime;
	return (unsigned long)((tv.tv_sec * 1000ul) + (tv.tv_usec / 1000ul));
#elif defined(_WIN32)
	return GetTickCount();
#elif
	return fakeTime;
#endif
}

template <class T>
vector<T> invert(vector<T> array, Size size)
{
	// init mx
	T** mx = new T*[size.width];
	for (int i = 0; i < size.width; i++)
	{
		mx[i] = new T[size.height];
	}

	// populate mx
	for (int i = 0; i < size.width; i++)
	{
		for (int j = 0; j < size.height; j++)
		{
			mx[i][j] = array[i*size.height + j];
		}
	}

	// invert into new array
	vector<T> newarray;
	for (int j = 0; j < size.height; j++)
	{
		for (int i = 0; i < size.width; i++)
		{
			newarray.push_back(mx[i][j]);
		}
	}

	return newarray;
}

template <class T>
vector<T> rotate(vector<T> array, Size size)
{
	// init mx
	T** mx = new T*[size.width];
	for (int i = 0; i < size.width; i++)
	{
		mx[i] = new T[size.height];
	}

	// populate mx
	for (int i = 0; i < size.width; i++)
	{
		for (int j = 0; j < size.height; j++)
		{
			mx[i][j] = array[i*size.height + j];
		}
	}

	// rotate into new array
	vector<T> newarray;
	for (int j = size.height-1; j >= 0; j--)
	{
		for (int i = 0; i < size.width; i++)
		{
			newarray.push_back(mx[i][j]);
		}
	}

	return newarray;
}

int main(int argc, char **argv)
{
	if (!strcmp(Email, "garry@kaspa.rov"))
	{
		cout << "Toltsd ki rendesen az Email-t a forrasban! :)" << endl;
		return 1;
	}

	VideoCapture input_video;
	if(!input_video.open("Chess.mp4"))
	{
		cout << "Could not play video!" << endl;
		return 1;
	}

	double fps = input_video.get(CV_CAP_PROP_FPS);
	int frn = (int)input_video.get(CV_CAP_PROP_FRAME_COUNT);

	namedWindow(Email, CV_WINDOW_NORMAL);
	resizeWindow(Email, (int)input_video.get(CV_CAP_PROP_FRAME_WIDTH), (int)input_video.get(CV_CAP_PROP_FRAME_HEIGHT));

	Mat logo = imread("hand.jpg");
	resize(logo, logo, Size(100, 100));
	rectangle(logo, Point(0,0), Point(logo.cols-1,logo.rows-1), Scalar(0,0,0), 2);

	Mat cap_img;
	Size patternSize(7, 7);
	vector<Point2f> corners;
	vector<Point2f> fixedCorners;
	vector<Point2f> rotatedCorners;
	vector<Point2f> previousCorners;

	for(int i=0; i<frn; i++)
	{ 
		long time = msec();

		input_video.read(cap_img);
		
		Mat logoPlacement = cap_img(Rect(cap_img.cols-logo.cols-20, 20, logo.cols, logo.rows));
		logo.copyTo(logoPlacement);

		bool patternFound = findChessboardCorners(cap_img, patternSize, corners);

		// setting initial grid
		reverse(corners.begin(), corners.end());
		auto invertedCorners = invert<Point2f>(corners, patternSize);
		corners = invertedCorners;

		rotatedCorners = corners;

		if (i == 0)
		{
			fixedCorners = corners;
			previousCorners = corners;
		}
		// rotating grid, if necessary
		else
		{
			bool needsRotate = true;
			while (needsRotate)
			{
				needsRotate = false;
				for (int j = 0; j < corners.size(); j++)
				{
					if (fabs(rotatedCorners[j].x - previousCorners[j].x) > 50.f ||
						fabs(rotatedCorners[j].y - previousCorners[j].y) > 50.f)
					{
						needsRotate = true;
					}
				}

				if (needsRotate)
				{
					fixedCorners = rotate<Point2f>(rotatedCorners, patternSize);
					rotatedCorners = fixedCorners;
				}
				else
				{
					fixedCorners = rotatedCorners;
				}
			}
			previousCorners = fixedCorners;
		}

		drawChessboardCorners(cap_img, patternSize, Mat(fixedCorners), patternFound);

		imshow(Email, cap_img);

		if (-1 != cvWaitKey(std::max(1l, (long)((1000.0) / fps) - (msec()-time)))) break;
	}

    return 0;  
}  
