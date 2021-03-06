#include <windows.h>

#include <gl/glut.h>

#include <math.h>
#include <time.h>
#include <stdio.h>

#pragma comment(lib, "Gdi32.lib")

#define ColoredVertex(c, v) { glColor3fv(c);  glVertex3fv(v); }

static GLfloat angle = 0.0f;
const GLfloat Pi = 3.1415926536f;

double DoCalFrequency()
{
     static int count;
     static double save;
     static clock_t last, current;
     double timegap;

     ++count;
     if( count <= 50 )
          return save;

     count = 0;
     last = current;
     current = clock();
     timegap = (current-last)/(double)CLK_TCK;
     save = 50.0/timegap;
     return save;
}

void selectFont(int size, int charset, const char* face)
{
    HFONT hFont = CreateFontA(size, 0, 0, 0, FW_MEDIUM, 0, 0, 0,
        charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);
    HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
    DeleteObject(hOldFont);
}

void drawString(const char* str)
{
     const int MAX_CHAR = 128;
    static int isFirstCall = 1;
    static GLuint lists;

    if( isFirstCall ) { // first call
          // make a table
        isFirstCall = 0;
 
        // generate MAX_CHAR size char
        lists = glGenLists(MAX_CHAR);

          selectFont(24, ANSI_CHARSET, "Comic Sans MS");
 
        // put MAX_CHAR to list
        wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);    

          //glEnable(GL_DEPTH_TEST);
    }
    // call list
    for(; *str!='\0'; ++str)
     {
        glCallList(lists + *str);
     }
}

void RenderCube(void)
{
     double FPS = DoCalFrequency();
     //printf("FPS = %f\n", FPS);
     char strFPS[20];
     sprintf(strFPS, "FPS = %f\n", FPS);    

     static int  list   = 0;
     static const int DIV_SIZE = 4;
     if( list  == 0 )
     {     //create table if dont exist
          GLfloat
          //cube
          //     PointCA[] = {-1.0f/DIV_SIZE, -1.0f/DIV_SIZE, -1.0f/DIV_SIZE},
          //     PointCB[] = {-1.0f/DIV_SIZE,  1.0f/DIV_SIZE, -1.0f/DIV_SIZE},
          //     PointCC[] = { 1.0f/DIV_SIZE,  1.0f/DIV_SIZE, -1.0f/DIV_SIZE},
          //     PointCD[] = { 1.0f/DIV_SIZE, -1.0f/DIV_SIZE, -1.0f/DIV_SIZE},
          //     PointCE[] = {-1.0f/DIV_SIZE, -1.0f/DIV_SIZE,  1.0f/DIV_SIZE},
          //     PointCF[] = {-1.0f/DIV_SIZE,  1.0f/DIV_SIZE,  1.0f/DIV_SIZE},
          //     PointCG[] = { 1.0f/DIV_SIZE,  1.0f/DIV_SIZE,  1.0f/DIV_SIZE},
          //     PointCH[] = { 1.0f/DIV_SIZE, -1.0f/DIV_SIZE,  1.0f/DIV_SIZE};
          //cuboid
               PointCA[] = {-2.0f/DIV_SIZE, -1.0f/DIV_SIZE, -0.5f/DIV_SIZE},
               PointCB[] = {-2.0f/DIV_SIZE,  1.0f/DIV_SIZE, -0.5f/DIV_SIZE},
               PointCC[] = { 2.0f/DIV_SIZE,  1.0f/DIV_SIZE, -0.5f/DIV_SIZE},
               PointCD[] = { 2.0f/DIV_SIZE, -1.0f/DIV_SIZE, -0.5f/DIV_SIZE},
               PointCE[] = {-2.0f/DIV_SIZE, -1.0f/DIV_SIZE,  0.5f/DIV_SIZE},
               PointCF[] = {-2.0f/DIV_SIZE,  1.0f/DIV_SIZE,  0.5f/DIV_SIZE},
               PointCG[] = { 2.0f/DIV_SIZE,  1.0f/DIV_SIZE,  0.5f/DIV_SIZE},
               PointCH[] = { 2.0f/DIV_SIZE, -1.0f/DIV_SIZE,  0.5f/DIV_SIZE};
          GLfloat
               ColorR[] = {1, 0, 0},
               ColorG[] = {0, 1, 0},
               ColorB[] = {0, 0, 1};
          list  = glGenLists(1);
          glNewList(list, GL_COMPILE);
          glBegin(GL_QUADS);
          // plane ABCD
          ColoredVertex(ColorR, PointCA);
          ColoredVertex(ColorR, PointCB);
          ColoredVertex(ColorR, PointCC);
          ColoredVertex(ColorR, PointCD);
          // plane EHGF
          ColoredVertex(ColorR, PointCE);
          ColoredVertex(ColorR, PointCH);
          ColoredVertex(ColorR, PointCG);
          ColoredVertex(ColorR, PointCF);

          // plane AEHD
          ColoredVertex(ColorG, PointCA);
          ColoredVertex(ColorG, PointCE);
          ColoredVertex(ColorG, PointCH);
          ColoredVertex(ColorG, PointCD);
          // plane AEHD
          ColoredVertex(ColorG, PointCB);
          ColoredVertex(ColorG, PointCF);
          ColoredVertex(ColorG, PointCG);
          ColoredVertex(ColorG, PointCC);

          // plane DHGC
          ColoredVertex(ColorB, PointCD);
          ColoredVertex(ColorB, PointCH);
          ColoredVertex(ColorB, PointCG);
          ColoredVertex(ColorB, PointCC);
          // plane AEFB
          ColoredVertex(ColorB, PointCA);
          ColoredVertex(ColorB, PointCE);
          ColoredVertex(ColorB, PointCF);
          ColoredVertex(ColorB, PointCB);

          glEnd();
          glEndList();
          glEnable(GL_DEPTH_TEST);
         
     }

     //quarts
     float quarts[][4] =
     {
0.99882, 0.01495, 0.02501, 0.03879,
0.99852, 0.02711, 0.02631, 0.03918,
0.99801, 0.03948, 0.02710, 0.04106,
0.99753, 0.04879, 0.02793, 0.04218,
0.99676, 0.06165, 0.02910, 0.04280,
0.99569, 0.07603, 0.03081, 0.04332,
0.99397, 0.09520, 0.03381, 0.04265,
0.99155, 0.11658, 0.03825, 0.04207,
0.98894, 0.13590, 0.04332, 0.04069,
0.98579, 0.15523, 0.05156, 0.03821,
0.98161, 0.18010, 0.05162, 0.03657,
0.97856, 0.19580, 0.05333, 0.03511,
0.97441, 0.21346, 0.06035, 0.03635,
0.96591, 0.24243, 0.07807, 0.04630,
0.95648, 0.27015, 0.09526, 0.05563,
0.94573, 0.29839, 0.10958, 0.06747,

0.99882, 0.01495, 0.02501, 0.03879,
0.99852, 0.02711, 0.02631, 0.03918,
0.99801, 0.03948, 0.02710, 0.04106,
0.99753, 0.04879, 0.02793, 0.04218,
0.99676, 0.06165, 0.02910, 0.04280,
0.99569, 0.07603, 0.03081, 0.04332,
0.99397, 0.09520, 0.03381, 0.04265,
0.99155, 0.11658, 0.03825, 0.04207,
0.98894, 0.13590, 0.04332, 0.04069,
0.98579, 0.15523, 0.05156, 0.03821,
0.98161, 0.18010, 0.05162, 0.03657,
0.97856, 0.19580, 0.05333, 0.03511,
0.97441, 0.21346, 0.06035, 0.03635,
0.96591, 0.24243, 0.07807, 0.04630,
0.95648, 0.27015, 0.09526, 0.05563,
0.94573, 0.29839, 0.10958, 0.06747,

0.99882, 0.01495, 0.02501, 0.03879,
0.99852, 0.02711, 0.02631, 0.03918,
0.99801, 0.03948, 0.02710, 0.04106,
0.99753, 0.04879, 0.02793, 0.04218,
0.99676, 0.06165, 0.02910, 0.04280,
0.99569, 0.07603, 0.03081, 0.04332,
0.99397, 0.09520, 0.03381, 0.04265,
0.99155, 0.11658, 0.03825, 0.04207,
0.98894, 0.13590, 0.04332, 0.04069,
0.98579, 0.15523, 0.05156, 0.03821,
0.98161, 0.18010, 0.05162, 0.03657,
0.97856, 0.19580, 0.05333, 0.03511,
0.97441, 0.21346, 0.06035, 0.03635,
0.96591, 0.24243, 0.07807, 0.04630,
0.95648, 0.27015, 0.09526, 0.05563,
0.94573, 0.29839, 0.10958, 0.06747,

0.99882, 0.01495, 0.02501, 0.03879,
0.99852, 0.02711, 0.02631, 0.03918,
0.99801, 0.03948, 0.02710, 0.04106,
0.99753, 0.04879, 0.02793, 0.04218,
0.99676, 0.06165, 0.02910, 0.04280,
0.99569, 0.07603, 0.03081, 0.04332,
0.99397, 0.09520, 0.03381, 0.04265,
0.99155, 0.11658, 0.03825, 0.04207,
0.98894, 0.13590, 0.04332, 0.04069,
0.98579, 0.15523, 0.05156, 0.03821,
0.98161, 0.18010, 0.05162, 0.03657,
0.97856, 0.19580, 0.05333, 0.03511,
0.97441, 0.21346, 0.06035, 0.03635,
0.96591, 0.24243, 0.07807, 0.04630,
0.95648, 0.27015, 0.09526, 0.05563,
0.94573, 0.29839, 0.10958, 0.06747,

     };
     //x= sin(angle/2)·xa
     //y= sin(angle/2)·ya
     //z= sin(angle/2)·za
     //w= cos(angle/2)

     static int num = 0;
     static int totalnum = sizeof(quarts)/4/4;
     num = (num + 1) % totalnum;

     float w = quarts[num][0];
     float x = quarts[num][1];
     float y = quarts[num][2];
     float z = quarts[num][3];

     angle = 2 * acos(w) * 180/Pi;
     float scale = sqrt(x*x + y*y + z*z);
     float xa = x / scale;
     float ya = y / scale;
     float za = z / scale;

     printf("angle=%.5f, x=%.5f, y=%.5f, z=%.5f\n", angle, xa, ya,za);


     // use list
     glClear(GL_COLOR_BUFFER_BIT  |  GL_DEPTH_BUFFER_BIT);
     glPushMatrix();
     //glRotatef(angle, 1, 1, 1);
     glRotatef(angle, xa, ya, za);
     glCallList(list);    
     glPopMatrix();

     glColor3f(1.0f, 1.0f, 1.0f);
     glRasterPos2f(-1.0f, -1.0f);
     drawString(strFPS);

     glutSwapBuffers();
}
void DoIdle(void)
{
     Sleep(50);
     //++angle;
     //if( angle >= 360.0f )
     //     angle = 0.0f;

     RenderCube();
}

void ResizeFunction(int Width, int Height)
{
     int CurrentWidth = Width;
     int CurrentHeight = Height;
     glViewport(0, 0, CurrentWidth, CurrentHeight);
}

int  main(int argc, char* argv[])
{
     static const int WIDTH = 800;
     static const int HEIGHT = 800;

     glutInit(&argc,  argv);
     glutInitDisplayMode(GLUT_RGBA  |  GLUT_DOUBLE);
     glutInitWindowPosition(100, 100);
     glutInitWindowSize(WIDTH, HEIGHT);
     glutCreateWindow("Sensor Show Cuboid");
     //glutReshapeFunc(&ResizeFunction);
     glutDisplayFunc(&RenderCube);
     glutIdleFunc(&DoIdle);
     glutMainLoop();
     return 0;
}