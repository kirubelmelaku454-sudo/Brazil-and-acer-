#include <GL/glut.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ===============================
// Global Transformation Variables
// ===============================
float gScale = 1.0f;
float gRotate = 0.0f;
float gTranslateX = 0.0f;
float gTranslateY = 0.0f;

float waveTime = 0.0f;

// =====================================================
// Wave function (Left to Right Cloth Motion)
// =====================================================
float waveOffset(float x)
{
    float strength = (x / 20.0f);
    return 0.45f * strength * sin((x * 1.2f) - waveTime);
}
// ---------------------------------------------------------
// Helper: Draw a mathematically perfect 5-pointed star
// ---------------------------------------------------------
void drawStar(float cx, float cy, float radius)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);

    for (int i = 0; i <= 10; ++i)
    {
        float angle = i * M_PI / 5.0f + M_PI / 2.0f;
        float r = (i % 2 == 0) ? radius : radius * 0.382f;
        glVertex2f(cx + r * cos(angle), cy + r * sin(angle));
    }

    glEnd();
}

// ---------------------------------------------------------
// Helper: Mini Vector Sans-Serif Font
// ---------------------------------------------------------
void drawChar(char c)
{
    switch (c)
    {
        case 'O':
            glBegin(GL_LINE_LOOP);
            glVertex2f(0.2, 0); glVertex2f(0.5, 0); glVertex2f(0.7, 0.2); glVertex2f(0.7, 0.8);
            glVertex2f(0.5, 1); glVertex2f(0.2, 1); glVertex2f(0, 0.8); glVertex2f(0, 0.2);
            glEnd();
            break;

        case 'R':
            glBegin(GL_LINE_STRIP);
            glVertex2f(0,0); glVertex2f(0,1); glVertex2f(0.5,1);
            glVertex2f(0.7,0.8); glVertex2f(0.7,0.6); glVertex2f(0.5,0.4); glVertex2f(0,0.4);
            glEnd();

            glBegin(GL_LINE_STRIP);
            glVertex2f(0.4,0.4); glVertex2f(0.7,0);
            glEnd();
            break;

        case 'D':
            glBegin(GL_LINE_LOOP);
            glVertex2f(0,0); glVertex2f(0,1); glVertex2f(0.4,1);
            glVertex2f(0.7,0.7); glVertex2f(0.7,0.3); glVertex2f(0.4,0);
            glEnd();
            break;

        case 'E':
            glBegin(GL_LINE_STRIP);
            glVertex2f(0.7,1); glVertex2f(0,1); glVertex2f(0,0); glVertex2f(0.7,0);
            glEnd();

            glBegin(GL_LINES);
            glVertex2f(0,0.5); glVertex2f(0.5,0.5);
            glEnd();
            break;

        case 'M':
            glBegin(GL_LINE_STRIP);
            glVertex2f(0,0); glVertex2f(0,1); glVertex2f(0.35,0.5);
            glVertex2f(0.7,1); glVertex2f(0.7,0);
            glEnd();
            break;

        case 'P':
            glBegin(GL_LINE_STRIP);
            glVertex2f(0,0); glVertex2f(0,1); glVertex2f(0.5,1);
            glVertex2f(0.7,0.8); glVertex2f(0.7,0.6); glVertex2f(0.5,0.4); glVertex2f(0,0.4);
            glEnd();
            break;

        case 'G':
            glBegin(GL_LINE_STRIP);
            glVertex2f(0.7,0.8); glVertex2f(0.5,1); glVertex2f(0.2,1);
            glVertex2f(0,0.8); glVertex2f(0,0.2); glVertex2f(0.2,0); glVertex2f(0.5,0);
            glVertex2f(0.7,0.2); glVertex2f(0.7,0.5); glVertex2f(0.4,0.5);
            glEnd();
            break;

        case 'S':
            glBegin(GL_LINE_STRIP);
            glVertex2f(0.7, 0.85); glVertex2f(0.5, 1); glVertex2f(0.2, 1);
            glVertex2f(0, 0.85); glVertex2f(0, 0.65); glVertex2f(0.2, 0.5); glVertex2f(0.5, 0.5);
            glVertex2f(0.7, 0.35); glVertex2f(0.7, 0.15); glVertex2f(0.5, 0); glVertex2f(0.2, 0);
            glVertex2f(0, 0.15);
            glEnd();
            break;
    }
}

// ---------------------------------------------------------
// Draw curved text (Thin thickness)
// ---------------------------------------------------------
void drawTextAlongCurve()
{
    const char* text = "ORDEM E PROGRESSO";
    glColor3f(0.0f, 0.608f, 0.227f);

    float currentAngle = 1.70f;
    float r = 8.12f;
    float scale = 0.25f;

    glLineWidth(1.0f);

    for (int i = 0; text[i] != '\0'; ++i)
    {
        float charWidth = 0.7f * scale;

        if (text[i] == ' ')
        {
            currentAngle -= (2.1f * scale) / r;
            continue;
        }

        float charAngle = currentAngle - (charWidth / 2.0f) / r;
        float x = 8.0f + r * cos(charAngle);
        float y = -1.0f + r * sin(charAngle);

        // Apply waving effect to text position
        y += waveOffset(x);

        glPushMatrix();
        glTranslatef(x, y, 0.0f);

        float deg = (charAngle - M_PI / 2.0f) * 180.0f / M_PI;
        glRotatef(deg, 0.0f, 0.0f, 1.0f);

        glTranslatef(-charWidth / 2.0f, 0.0f, 0.0f);
        glScalef(scale, scale, 1.0f);

        drawChar(text[i]);

        glPopMatrix();

        currentAngle -= charWidth / r;
        currentAngle -= (0.7f * scale) / r;
    }

    glLineWidth(1.0f);
}

// ---------------------------------------------------------
// Stars
// ---------------------------------------------------------
struct StarData { float x, y, r; };

StarData stars[27] = {
    {11.04f, 7.20f, 0.15f},
    {9.8f, 5.5f, 0.12f}, {9.8f, 4.2f, 0.15f}, {9.2f, 4.8f, 0.12f},
    {10.4f, 5.0f, 0.09f}, {10.0f, 4.5f, 0.07f},
    {11.5f, 5.2f, 0.15f}, {11.2f, 5.6f, 0.09f}, {11.6f, 5.8f, 0.09f},
    {12.0f, 5.7f, 0.09f}, {11.8f, 4.8f, 0.09f}, {12.2f, 4.4f, 0.09f},
    {12.5f, 4.8f, 0.12f}, {12.3f, 5.2f, 0.09f},
    {8.2f, 5.2f, 0.15f}, {7.6f, 5.5f, 0.12f}, {8.4f, 4.5f, 0.12f},
    {7.8f, 4.3f, 0.12f}, {8.6f, 4.0f, 0.09f},
    {11.0f, 4.2f, 0.12f}, {10.6f, 3.8f, 0.09f}, {11.2f, 3.7f, 0.09f},
    {8.8f, 3.8f, 0.15f}, {7.2f, 6.0f, 0.15f},
    {7.8f, 6.0f, 0.09f}, {8.4f, 6.0f, 0.09f},
    {10.0f, 3.7f, 0.05f}
};

// ---------------------------------------------------------
// Display
// ---------------------------------------------------------
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glPushMatrix();

    // Translation first
    glTranslatef(gTranslateX, gTranslateY, 0.0f);

    // Rotate and scale around center of flag (10,7)
    glTranslatef(10.0f, 7.0f, 0.0f);
    glRotatef(gRotate, 0.0f, 0.0f, 1.0f);
    glScalef(gScale, gScale, 1.0f);
    glTranslatef(-10.0f, -7.0f, 0.0f);

    // 1. Green background (waving)
    glColor3f(0.0f, 0.608f, 0.227f);
    glBegin(GL_QUAD_STRIP);
    for (float x = 0.0f; x <= 20.0f; x += 0.2f)
    {
        glVertex2f(x, 0.0f + waveOffset(x));
        glVertex2f(x, 14.0f + waveOffset(x));
    }
    glEnd();

    // 2. Yellow rhombus (waving)
    glColor3f(0.996f, 0.875f, 0.0f);
    glBegin(GL_POLYGON);
    glVertex2f(10.0f, 1.7f + waveOffset(10.0f));
    glVertex2f(18.3f, 7.0f + waveOffset(18.3f));
    glVertex2f(10.0f, 12.3f + waveOffset(10.0f));
    glVertex2f(1.7f, 7.0f + waveOffset(1.7f));
    glEnd();

    // stencil
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // 3. blue circle (waving)
    glColor3f(0.0f, 0.153f, 0.463f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(10.0f, 7.0f + waveOffset(10.0f));

    for (int i = 0; i <= 360; ++i)
    {
        float angle = i * M_PI / 180.0f;
        float x = 10.0f + 3.5f * cos(angle);
        float y = 7.0f + 3.5f * sin(angle);
        glVertex2f(x, y + waveOffset(x));
    }
    glEnd();

    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // 4. white band (waving)
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 600; i <= 1400; i += 5)
    {
        float x = i / 100.0f;
        float dx = x - 8.0f;
        if (dx > 8.5f || dx < -8.5f) continue;

        float outer_val = 8.5f * 8.5f - dx * dx;
        float inner_val = 8.0f * 8.0f - dx * dx;

        float y_outer = -1.0f + sqrt(outer_val > 0 ? outer_val : 0);
        float y_inner = -1.0f + sqrt(inner_val > 0 ? inner_val : 0);

        glVertex2f(x, y_outer + waveOffset(x));
        glVertex2f(x, y_inner + waveOffset(x));
    }
    glEnd();

    // 5. text (waving)
    drawTextAlongCurve();

    // 6. stars (waving)
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < 27; ++i)
    {
        drawStar(stars[i].x, stars[i].y + waveOffset(stars[i].x), stars[i].r);
    }

    glDisable(GL_STENCIL_TEST);

    glPopMatrix();

    glutSwapBuffers();
}

// ---------------------------------------------------------
// Timer (animation)
// ---------------------------------------------------------
void timer(int value)
{
    waveTime += 0.10f; // wave speed (left -> right movement)
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// ---------------------------------------------------------
// Keyboard Controls
// ---------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case '1': gScale += 0.1f; break;
        case '2': gScale -= 0.1f; if (gScale < 0.2f) gScale = 0.2f; break;
        case '3': gRotate += 10.0f; break;
        case '4': gTranslateX += 0.5f; break;

        case '0':
            gScale = 1.0f;
            gRotate = 0.0f;
            gTranslateX = 0.0f;
            gTranslateY = 0.0f;
            break;
    }

    glutPostRedisplay();
}

// ---------------------------------------------------------
// Reshape
// ---------------------------------------------------------
void reshape(int w, int h)
{
    if (h == 0) h = 1;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-2.0f, 22.0f, -2.0f, 16.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ---------------------------------------------------------
// Init
// ---------------------------------------------------------
void initGL()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

// ---------------------------------------------------------
// Main
// ---------------------------------------------------------
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE | GLUT_STENCIL);

    glutInitWindowSize(900, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Brazil Flag - Left to Right Waving");

    initGL();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
