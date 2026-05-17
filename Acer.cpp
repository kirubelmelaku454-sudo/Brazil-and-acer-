/*

 *  Acer Logo - OpenGL / FreeGLUT Renderer
 *
 *  Correctly renders the Acer logo with:
 *   - Open counter (hole) in the letter 'a'
 *   - Open counter (hole) in the letter 'e'
 *
 *  The SVG data contains 3 sub-paths:
 *    [0]  outer body of the full word  (wound clockwise in SVG)
 *    [1]  counter hole inside 'a'      (wound counter-clockwise)
 *    [2]  counter hole inside 'e'      (wo
 *
 *  All three are submitted as SEPARATE CONTOURS inside a single
 *  GLU tessellator polygon call with GLU_TESS_WINDING_ODD so
 *  the inner contours are automatically subtracted (holes).
 
 *  Compile (Linux / macOS):
 *    g++ -o acer_logo acer_logo.cpp -lGL -lGLU -lglut -lm
 *    or with FreeGLUT:
 *    g++ -o acer_logo acer_logo.cpp -lGL -lGLU -lfreeglut -lm
 *
 *  Compile (Windows / MinGW):
 *    g++ -o acer_logo acer_logo.cpp -lfreeglut -lopengl32 -lglu32
 *
 *  Run:
 *    ./acer_logo
 *
 *  Keyboard controls:
 *    +  /  =   Zoom in
 *    -         Zoom out
 *    r         Toggle rotation animation
 *    0         Reset 
 *    1         Color: Red
 *    2         Color: Charcoal
 *    3         Color: Blue
 *    4         Color: Acer Green (original)
 *    q / ESC   Quit

 */

#ifdef _WIN32
  #include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cctype>

// ============================================================
//  Constants
// ============================================================
static const int WIN_W        = 1000;
static const int WIN_H        = 700;
static const int BEZIER_STEPS = 48;   // curve smoothness

struct Pt { double x, y; };

struct SubPath {
    std::vector<Pt> pts;
    bool closed;
};

static std::vector<SubPath> g_paths; 

static float g_zoom     = 1.0f;
static float g_angle    = 0.0f;
static bool  g_rotating = false;
static float g_colorR = 120.0f / 255.0f;
static float g_colorG = 190.0f / 255.0f;
static float g_colorB =  32.0f / 255.0f;

struct Token { bool isCmd; char cmd; double num; };

static std::vector<Token> tokenise(const std::string& d)
{
    std::vector<Token> toks;
    const char* p = d.c_str();
    while (*p) {
        if (std::isspace((unsigned char)*p) || *p == ',') { ++p; continue; }
        if (std::isalpha((unsigned char)*p)) {
            Token t; t.isCmd=true; t.cmd=*p; t.num=0;
            toks.push_back(t); ++p; continue;
        }
        if (*p=='-'||*p=='+'||std::isdigit((unsigned char)*p)||*p=='.') {
            char* end;
            double v = std::strtod(p, &end);
            if (end==p){++p;continue;}
            Token t; t.isCmd=false; t.cmd=0; t.num=v;
            toks.push_back(t); p=end; continue;
        }
        ++p;
    }
    return toks;
}

static void sampleCubic(std::vector<Pt>& out,
                        Pt p0,Pt p1,Pt p2,Pt p3,int steps)
{
    for(int i=1;i<=steps;++i){
        double t=(double)i/steps, mt=1.0-t;
        out.push_back({
            mt*mt*mt*p0.x+3*mt*mt*t*p1.x+3*mt*t*t*p2.x+t*t*t*p3.x,
            mt*mt*mt*p0.y+3*mt*mt*t*p1.y+3*mt*t*t*p2.y+t*t*t*p3.y
        });
    }
}

static void sampleQuadratic(std::vector<Pt>& out,
                             Pt p0,Pt p1,Pt p2,int steps)
{
    for(int i=1;i<=steps;++i){
        double t=(double)i/steps, mt=1.0-t;
        out.push_back({
            mt*mt*p0.x+2*mt*t*p1.x+t*t*p2.x,
            mt*mt*p0.y+2*mt*t*p1.y+t*t*p2.y
        });
    }
}

static void parseSVGPath(const std::string& d)
{
    g_paths.clear();
    std::vector<Token> toks = tokenise(d);
    if(toks.empty()) return;

    size_t i=0;
    auto hasNum=[&](){ return i<toks.size()&&!toks[i].isCmd; };
    auto getNum=[&]()->double{
        if(i>=toks.size()||toks[i].isCmd) throw std::runtime_error("expected number");
        return toks[i++].num;
    };

    Pt cur{0,0}, start{0,0}, lastCtrl{0,0};
    char lastCmd=0;
    SubPath* sp=nullptr;

    auto newSubPath=[&](Pt from){
        g_paths.push_back(SubPath());
        sp=&g_paths.back();
        sp->closed=false;
        sp->pts.push_back(from);
    };

    while(i<toks.size())
    {
        if(!toks[i].isCmd){
        } else {
            lastCmd=toks[i++].cmd;
        }

        char cmd=lastCmd;
        bool rel=(std::islower((unsigned char)cmd)!=0);
        char ABS=(char)std::toupper((unsigned char)cmd);

        if(ABS=='M'){
            double mx=getNum(),my=getNum();
            if(rel&&sp){mx+=cur.x;my+=cur.y;}
            cur={mx,my}; start=cur;
            newSubPath(cur);
            lastCtrl=cur;
            lastCmd=rel?'l':'L';
            continue;
        }
        if(ABS=='Z'){
            if(sp){sp->closed=true; sp->pts.push_back(start);}
            cur=start; lastCtrl=cur; lastCmd=0;
            continue;
        }
        if(!sp) newSubPath(cur);

        if(ABS=='L'){
            while(hasNum()){
                double x=getNum(),y=getNum();
                if(rel){x+=cur.x;y+=cur.y;}
                cur={x,y}; sp->pts.push_back(cur);
            }
            lastCtrl=cur;
        }
        else if(ABS=='H'){
            while(hasNum()){
                double x=getNum();
                if(rel)x+=cur.x;
                cur.x=x; sp->pts.push_back(cur);
            }
            lastCtrl=cur;
        }
        else if(ABS=='V'){
            while(hasNum()){
                double y=getNum();
                if(rel)y+=cur.y;
                cur.y=y; sp->pts.push_back(cur);
            }
            lastCtrl=cur;
        }
        else if(ABS=='C'){
            while(hasNum()){
                double x1=getNum(),y1=getNum(),
                       x2=getNum(),y2=getNum(),
                       x =getNum(),y =getNum();
                if(rel){x1+=cur.x;y1+=cur.y;x2+=cur.x;y2+=cur.y;x+=cur.x;y+=cur.y;}
                Pt p1={x1,y1},p2={x2,y2},p3={x,y};
                sampleCubic(sp->pts,cur,p1,p2,p3,BEZIER_STEPS);
                lastCtrl=p2; cur=p3;
            }
        }
        else if(ABS=='S'){
            while(hasNum()){
                double x2=getNum(),y2=getNum(),x=getNum(),y=getNum();
                if(rel){x2+=cur.x;y2+=cur.y;x+=cur.x;y+=cur.y;}
                Pt p1={2*cur.x-lastCtrl.x,2*cur.y-lastCtrl.y};
                Pt p2={x2,y2},p3={x,y};
                sampleCubic(sp->pts,cur,p1,p2,p3,BEZIER_STEPS);
                lastCtrl=p2; cur=p3;
            }
        }
        else if(ABS=='Q'){
            while(hasNum()){
                double x1=getNum(),y1=getNum(),x=getNum(),y=getNum();
                if(rel){x1+=cur.x;y1+=cur.y;x+=cur.x;y+=cur.y;}
                Pt p1={x1,y1},p2={x,y};
                sampleQuadratic(sp->pts,cur,p1,p2,BEZIER_STEPS);
                lastCtrl=p1; cur=p2;
            }
        }
        else if(ABS=='T'){
            while(hasNum()){
                double x=getNum(),y=getNum();
                if(rel){x+=cur.x;y+=cur.y;}
                Pt p1={2*cur.x-lastCtrl.x,2*cur.y-lastCtrl.y},p2={x,y};
                sampleQuadratic(sp->pts,cur,p1,p2,BEZIER_STEPS);
                lastCtrl=p1; cur=p2;
            }
        }
        else { if(i<toks.size())++i; }
    }
}

static void CALLBACK tessBegin (GLenum t)  { glBegin(t); }
static void CALLBACK tessEnd   ()          { glEnd(); }
static void CALLBACK tessVertex(void* d)   { double* v=(double*)d; glVertex2d(v[0],v[1]); }
static void CALLBACK tessError (GLenum e)  { fprintf(stderr,"GLU Tess: %s\n",gluErrorString(e)); }
static void CALLBACK tessCombine(double c[3],void*[4],float[4],void** out){
    double* v=new double[3]; v[0]=c[0];v[1]=c[1];v[2]=0; *out=v;
}

// ============================================================
//  Core tessellation: ALL sub-paths as one multi-contour polygon
//
//  KEY FIX:  We do NOT tessellate each sub-path separately.
//  Instead we open ONE polygon and add EVERY closed sub-path
//  as its own contour.  GLU_TESS_WINDING_ODD then treats
//  the counter-wound inner paths (the 'a' and 'e' counters)
//  as holes automatically — no coordinate changes needed.
// ============================================================
static void tessellateAll()
{
    // Collect only closed paths
    std::vector<const SubPath*> closed;
    for(const auto& sp : g_paths)
        if(sp.closed && sp.pts.size()>=3)
            closed.push_back(&sp);
    if(closed.empty()) return;
    std::vector< std::vector< std::vector<double> > > vdata(closed.size());
    for(size_t c=0;c<closed.size();++c){
        const SubPath& sp=*closed[c];
        vdata[c].resize(sp.pts.size(), std::vector<double>(3,0.0));
        for(size_t k=0;k<sp.pts.size();++k){
            vdata[c][k][0]=sp.pts[k].x;
            vdata[c][k][1]=sp.pts[k].y;
            vdata[c][k][2]=0.0;
        }
    }

    GLUtesselator* tess=gluNewTess();
    gluTessCallback(tess,GLU_TESS_BEGIN,   (GLvoid(*)())tessBegin);
    gluTessCallback(tess,GLU_TESS_END,     (GLvoid(*)())tessEnd);
    gluTessCallback(tess,GLU_TESS_VERTEX,  (GLvoid(*)())tessVertex);
    gluTessCallback(tess,GLU_TESS_ERROR,   (GLvoid(*)())tessError);
    gluTessCallback(tess,GLU_TESS_COMBINE, (GLvoid(*)())tessCombine);

    gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

    gluTessBeginPolygon(tess, nullptr);
    for(size_t c=0;c<closed.size();++c){
        gluTessBeginContour(tess);
        for(size_t k=0;k<closed[c]->pts.size();++k)
            gluTessVertex(tess, vdata[c][k].data(), vdata[c][k].data());
        gluTessEndContour(tess);
    }
    gluTessEndPolygon(tess);
    gluDeleteTess(tess);
}

static void getBounds(double& x0,double& y0,double& x1,double& y1)
{
    x0=y0=1e9; x1=y1=-1e9;
    for(const auto& sp:g_paths)
        for(const auto& pt:sp.pts){
            x0=std::min(x0,pt.x); y0=std::min(y0,pt.y);
            x1=std::max(x1,pt.x); y1=std::max(y1,pt.y);
        }
}

static void drawLogo()
{
    glColor3f(g_colorR, g_colorG, g_colorB);

    tessellateAll();

    for(const auto& sp:g_paths){
        if(!sp.closed){
            glLineWidth(2.0f);
            glBegin(GL_LINE_STRIP);
            for(const auto& pt:sp.pts) glVertex2d(pt.x,pt.y);
            glEnd();
        }
    }
}
static void scaleLogo    (float f)        { glScalef(f,f,1.0f); }
static void translateLogo(float x,float y){ glTranslatef(x,y,0.0f); }
static void rotateLogo   (float a)        { glRotatef(a,0.0f,0.0f,1.0f); }

static void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT,    GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);
}

static void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    double minX,minY,maxX,maxY;
    getBounds(minX,minY,maxX,maxY);
    double cx=(minX+maxX)*0.5, cy=(minY+maxY)*0.5;
    double w=maxX-minX, h=maxY-minY;
    if(w<1e-9||h<1e-9){glutSwapBuffers();return;}

    int vw=glutGet(GLUT_WINDOW_WIDTH);
    int vh=glutGet(GLUT_WINDOW_HEIGHT);
    double s=std::min((vw*0.82)/w,(vh*0.55)/h);

    glTranslatef(vw*0.5f, vh*0.5f, 0.0f);
    rotateLogo(g_angle);
    scaleLogo(g_zoom);
    glScaled(s,s,1.0);
    glScalef(1.0f,-1.0f,1.0f);   // flip Y: SVG down = OpenGL up
    glTranslated(-cx,-cy,0.0);

    drawLogo();
    glutSwapBuffers();
}

static void reshape(int w,int h)
{
    if(h==0)h=1;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0,(double)w,0.0,(double)h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ============================================================
//  Timer  (~60 fps rotation)
// ============================================================
static void timer(int)
{
    if(g_rotating){
        g_angle+=0.5f;
        if(g_angle>=360.0f)g_angle-=360.0f;
        glutPostRedisplay();
    }
    glutTimerFunc(16,timer,0);
}
static void keyboard(unsigned char key,int,int)
{
    switch(key)
    {
        case '+': case '=': g_zoom*=1.1f; break;
        case '-':           g_zoom/=1.1f; break;
        case 'r': case 'R': g_rotating=!g_rotating; break;
        case '0': g_zoom=1.0f;g_angle=0.0f;g_rotating=false; break;

        case '1':
            g_colorR=232.0f/255.0f; g_colorG=17.0f/255.0f; g_colorB=45.0f/255.0f;
            printf("Color: Red (#E8112D)\n"); break;

        case '2':
            g_colorR=54.0f/255.0f; g_colorG=69.0f/255.0f; g_colorB=79.0f/255.0f;
            printf("Color: Charcoal (#36454F)\n"); break;

        case '3':
            g_colorR=0.0f; g_colorG=120.0f/255.0f; g_colorB=212.0f/255.0f;
            printf("Color: Blue (#0078D4)\n"); break;

        case '4':
            g_colorR=120.0f/255.0f; g_colorG=190.0f/255.0f; g_colorB=32.0f/255.0f;
            printf("Color: Acer Green (#78BE20)\n"); break;

        case 'q': case 'Q': case 27: exit(0); break;
        default: break;
    }
    glutPostRedisplay();
}

// ============================================================
//  SVG path data  (from the provided Inkscape .d file)
//  Three sub-paths separated by 'z m':
//    1st  — outer body of "acer"
//    2nd  — counter hole inside 'a'
//    3rd  — counter hole inside 'e'
// ============================================================
static const char* SVG_PATH_DATA =
/* outer body */
"m 75.040585,170.98977 "
"c -6.861878,-1.86483 -9.20721,-6.64363 -6.726359,-13.70552 "
"  1.088198,-3.09762 4.110238,-6.1949 7.544341,-7.73216 "
"  1.140857,-0.51071 5.808002,-1.75409 10.371432,-2.76309 "
"  8.729738,-1.93016 9.334287,-2.27574 9.334287,-5.33561 "
"  0,-2.0794 -1.80282,-3.81746 -4.48111,-4.32015 "
"  -1.300219,-0.24403 -5.417516,-0.10592 -9.149547,0.30693 "
"  -3.732032,0.41284 -6.85976,0.63963 -6.950508,0.50397 "
"  -0.412106,-0.61608 1.054107,-5.16543 2.007851,-6.22994 "
"  0.918689,-1.02539 2.030937,-1.22821 8.261748,-1.50658 "
"  8.030177,-0.35878 12.044283,0.33135 14.678256,2.52352 "
"  2.004914,1.66862 3.186334,4.82296 3.004004,8.02055 "
"  -0.15578,2.73198 0.14643,2.95408 0.95321,0.70046 "
"  0.86918,-2.42788 3.93205,-6.21294 6.49329,-8.02431 "
"  3.24282,-2.29339 8.15998,-3.52529 13.07182,-3.27487 "
"  3.75134,0.19125 4.20209,0.34461 4.92196,1.67467 "
"  0.46528,0.85962 0.79245,2.42507 0.79245,3.79157 "
"v 2.32743 "
"l -5.9193,-0.0705 "
"c -5.91464,-0.0704 -5.92155,-0.0687 -8.73722,2.0026 "
"  -3.98624,2.93254 -5.87096,7.24075 -6.70214,15.3201 "
"  -0.73147,7.1102 2.6034,9.36699 12.68653,8.58532 "
"l 5.2256,-0.4051 -0.34703,-2.7769 "
"c -0.4395,-3.51695 0.40431,-9.86247 1.97228,-14.83159 "
"  1.44484,-4.57893 5.06074,-10.03436 8.04414,-12.13649 "
"  7.64593,-5.38739 21.83191,-4.51835 24.6566,1.51047 "
"  0.35734,0.76267 0.6497,2.21177 0.6497,3.22021 "
"v 1.83352 "
"l 1.8936,-2.95443 "
"c 3.45217,-5.38614 8.24274,-7.46205 16.365,-7.09151 "
"  5.59997,0.25548 6.11829,0.77801 5.11929,5.16102 "
"l -0.61262,2.68779 -5.88577,0.22437 "
"c -6.68472,0.25481 -8.24527,1.0135 -9.21399,4.47946 "
"  -0.31144,1.11432 -1.62645,7.58451 -2.92226,14.37822 "
"  -1.2958,6.79371 -2.52632,12.84058 -2.7345,13.43752 "
"  -0.31767,0.91086 -0.97639,1.08533 -4.09764,1.08533 "
"  -2.73078,0 -3.71912,-0.2094 -3.71912,-0.78798 "
"  0,-0.43337 1.02677,-6.1435 2.28172,-12.68916 "
"  1.25494,-6.54564 2.28172,-12.10659 2.28172,-12.35767 "
"  0,-0.25106 -0.60674,0.40544 -1.34829,1.45889 "
"  -2.9717,4.22162 -6.19578,5.70244 -16.52807,7.5914 "
"  -9.65587,1.76529 -9.22932,1.59115 -8.89694,3.63248 "
"  0.78705,4.83391 3.70019,5.93415 14.07735,5.31676 "
"l 7.00976,-0.41703 -0.27165,1.61804 "
"c -1.04375,6.21693 -1.15321,6.27821 -11.25874,6.30346 "
"  -9.59903,0.0239 -11.96626,-0.67989 -14.55309,-4.3271 "
"  -0.84424,-1.1903 -1.60221,-2.16418 -1.68437,-2.16418 "
"  -0.0822,0 -0.14943,0.7104 -0.14943,1.57866 "
"  0,0.86828 -0.43918,2.23522 -0.97595,3.03766 "
"  -0.8136,1.21628 -1.58505,1.53485 -4.6375,1.91496 "
"  -4.88859,0.60877 -12.01004,0.0125 -14.22339,-1.19048 "
"  -0.9848,-0.53536 -2.36003,-1.57957 -3.05606,-2.32048 "
"  -1.43619,-1.52883 -2.80559,-5.54159 -2.81913,-8.26092 "
"  -0.0129,-2.57811 -0.527875,-1.36291 -1.591563,3.7552 "
"  -0.503675,2.42351 -1.242729,4.84873 -1.642342,5.38937 "
"  -0.399614,0.54063 -1.937729,1.28886 -3.418035,1.66269 "
"  -3.61603,0.9132 -15.872929,1.33788 -18.444306,0.63907 "
"z "
/* counter hole inside 'a' */
"m 13.164238,-6.61051 "
"  2.804538,-0.39597 1.236015,-5.91133 "
"c 0.679807,-3.25123 1.149624,-6.04047 1.044037,-6.19831 "
"  -0.266135,-0.39787 -13.872498,2.39512 -15.149131,3.10966 "
"  -2.64565,1.48079 -3.744142,5.37089 -2.132391,7.55144 "
"  1.421029,1.92253 6.357145,2.669 12.196932,1.84451 "
"z "
/* counter hole inside 'e' */
"m 53.616047,-15.74753 "
"c 8.67177,-1.80797 11.93194,-4.21306 11.24636,-8.29664 "
"  -0.33603,-2.00144 -2.0725,-2.98167 -5.86611,-3.31135 "
"  -6.76998,-0.58832 -10.86902,2.52988 -12.64294,9.61773 "
"  -1.02118,4.08026 -1.02453,3.9668 0.10631,3.59474 "
"  0.51339,-0.16889 3.73372,-0.89091 7.1563,-1.60448 "
"z";

int main(int argc, char* argv[])
{
    parseSVGPath(SVG_PATH_DATA);

    if(g_paths.empty()){
        fprintf(stderr,"ERROR: No paths parsed.\n");
        return 1;
    }
    fprintf(stdout,"Parsed %zu sub-path(s).\n", g_paths.size());

    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_MULTISAMPLE);
    glutInitWindowSize(WIN_W,WIN_H);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Acer Logo - OpenGL");

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16,timer,0);

    printf("Controls:\n"
           "  +/=    Zoom in\n"
           "  -      Zoom out\n"
           "  r      Toggle rotation\n"
           "  0      Reset view\n"
           "  1      Color: Red\n"
           "  2      Color: Charcoal\n"
           "  3      Color: Blue\n"
           "  4      Color: Acer Green (original)\n"
           "  q/ESC  Quit\n");

    glutMainLoop();
    return 0;
}
