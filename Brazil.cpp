//braziel flag
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
