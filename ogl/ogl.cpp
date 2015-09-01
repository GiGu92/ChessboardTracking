//=============================================================================================
// Nemhivatalos szamitogepes grafika hazi gyakorlo keret 2015, beugro.
// Az Email definiciot ertelemszeruen toltsd ki.
// Tilos:
// - kozizlest serteni
// - rosszindulatu kodot hasznalni (adatgyujtes/tovabbitas, fajlok megvaltoztatasa, a tobbi program befolyasolasa, DOS-olas, stb.)
// ---------------------------------------------------------------------------------------------
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
#define Email   "garry@kaspa.rov"
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot a sajat szorakoztatasomra, kepemre es hasonlatossagomra keszitettem.
// Tisztaban vagyok azzal, hogy ami egyszer felkerult az internetre, azt onnan
// szinte lehetetlen eltuntetni, igy ha jelen kodomat nyilvanos helyen kozzeteszem,
// az adott esetben plagiumgyanut kelthet a hivatalos hazi feladat ertekelesekor.
//=============================================================================================

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
// MsWindows-on ez is kell
#include <windows.h>
#pragma warning(disable: 4996)
#endif // Win32 platform

#include <GL/gl.h>
#include <GL/glu.h>
// A GLUT-ot le kell tolteni: http://www.opengl.org/resources/libraries/glut/
#include <GL/glut.h>
#include <opencv2/opencv.hpp>

#include <vector>

using namespace cv;
using namespace std;

class Knight
{
public:
	Knight():angle(0) {}

	void Simulate(float ts, float te)
	{
		angle = fmod(angle + (te-ts) * 50.0, 360.0);
	}

	void Init()
	{
		logo = imread("knight.png");
		if (logo.rows && logo.cols)
		{
			cvtColor(logo, logo, CV_RGB2GRAY);
			Canny(logo, logo, 100, 200, 3);
			findContours(logo, silhouette, noArray(), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(-logo.cols/2, -logo.rows/2));
		}
	}

	void Draw()
	{
		glColor3f(0.9, 0.9, 0.9);
		glLineWidth(3);
		glPushMatrix();
			glTranslatef(100, 100, 0);
			glRotatef(angle, 0, 1, 0);
			glRotatef(fmod(angle*2.0, 360), 0, 0, 1);
			glRotatef(fmod(angle*4.0, 360), 1, 0, 0);
			glBegin(GL_LINE_LOOP);
			for (unsigned i=0; i<silhouette[0].size(); i++)
			{
				glVertex2f(silhouette[0][i].x, silhouette[0][i].y);
			}
			glEnd();
		glPopMatrix();
	}

private:
		float angle;
		Mat logo;
		vector<vector<Point> > silhouette;
} theKnight;

void simulateWorld()
{
	static float tstart = 0;
    float tend = glutGet(GLUT_ELAPSED_TIME)/1000.0;

	static const float dt = 0.05;
    for (float ts = tstart; ts < tend; ts += dt)
	{
        float te;
        if (tend >= ts + dt)
		{
            te = ts + dt;
        } else
		{
            te = tend;
        }

		theKnight.Simulate(ts, te);
	}

	tstart = tend;
}

int screenWidth;
int screenHeight;
cv::Mat capImg;
int frameDuration, numFrames;
cv::VideoCapture inputVideo;

void readVideoFrame()
{
	static int lastReadTime = 0, numReadFrames=0;

	int actualTime = glutGet(GLUT_ELAPSED_TIME);

	if (actualTime >= lastReadTime+frameDuration)
	{
		if (numReadFrames >= numFrames)
		{
			lastReadTime = numReadFrames = 0;
			inputVideo.set(CV_CAP_PROP_POS_FRAMES, 0);
		}
		inputVideo.read(capImg);
		cv::resize(capImg, capImg, cv::Size(screenWidth, screenHeight));
		cv::flip(capImg, capImg, 0);

		lastReadTime = actualTime;
		numReadFrames++;
	}
}

void onDisplay() 
{
	readVideoFrame();
    simulateWorld();

	glDrawPixels(screenWidth, screenHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, capImg.data);
	theKnight.Draw();

	glutSwapBuffers();
}

void onInitialization() 
{
	glViewport(0, 0, screenWidth, screenHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
	glOrtho(0, screenWidth, 0, screenHeight, -500, 500);
	theKnight.Init();
}

void onKeyboard(unsigned char key, int x, int y) 
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	}
	glutPostRedisplay();
}

void onMouse(int button, int state, int x, int y)
{
	if (state != GLUT_DOWN) return;

	if (button == GLUT_LEFT_BUTTON ) glutSetWindowTitle("left button");
	if (button == GLUT_RIGHT_BUTTON) glutSetWindowTitle("right button");

	glutPostRedisplay();
}

void onIdle() 
{
	simulateWorld();
	glutPostRedisplay();
}

void doNothing(){}

int main(int argc, char **argv) 
{
	glutInit(&argc, argv); // GLUT inicializalasa

	glutInitWindowSize(600, 600); // Alkalmazas ablak kezdeti merete, csak hogy legyen mire irni a hibauzenetet, ha nem lehet betolteni a videot
	glutInitWindowPosition(100, 100); // Az elozo alkalmazas ablakhoz kepest hol tunik fel
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); // 8 bites R,G,B,A + dupla buffer + melyseg buffer

	if (!inputVideo.open("Chess.mp4"))
	{
		glutCreateWindow("Can't play video!");
		glutDisplayFunc(doNothing);
		glutMainLoop();
		return 1;
	}

	screenWidth  = (int)inputVideo.get(CV_CAP_PROP_FRAME_WIDTH);
	screenHeight = (int)inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT);
	numFrames    = (int)inputVideo.get(CV_CAP_PROP_FRAME_COUNT);
	frameDuration = 1000.0 / inputVideo.get(CV_CAP_PROP_FPS);

	glutInitWindowSize(screenWidth, screenHeight); // Alkalmazas ablak kezdeti merete

	if (strcmp(Email, "garry@kaspa.rov"))
	{
		glutCreateWindow(Email); // Alkalmazas ablak megszuletik es megjelenik a kepernyon

		glMatrixMode(GL_MODELVIEW); // A MODELVIEW transzformaciot egysegmatrixra inicializaljuk
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION); // A PROJECTION transzformaciot egysegmatrixra inicializaljuk
		glLoadIdentity();

		onInitialization(); // Az altalad irt inicializalast lefuttatjuk

		glutDisplayFunc(onDisplay); // Esemenykezelok regisztralasa
		glutIdleFunc(onIdle);
		glutKeyboardFunc(onKeyboard);
		glutMouseFunc(onMouse);
	}
	else
	{
		glutCreateWindow("Toltsd ki rendesen az Email-t a forrasban! :)");
		glutDisplayFunc(doNothing); 
	}

    glutMainLoop(); // Esemenykezelo hurok

    return 0;
}
