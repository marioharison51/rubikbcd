#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char up[9];
    char down[9];
    char front[9];
    char back[9];
    char left[9];
    char right[9];
} RubikCube;

#define EXPORT __attribute__((visibility("default")))

EXPORT void reset_cube(RubikCube *cube) {
    int i;
    for(i=0; i<9; i++) {
        cube->up[i] = 'W';
        cube->down[i] = 'Y';
        cube->front[i] = 'G';
        cube->back[i] = 'B';
        cube->left[i] = 'O';
        cube->right[i] = 'R';
    }
}

void rotate_face(char f[9]) {
    char t[9];
    int i;
    for(i=0; i<9; i++) t[i] = f[i];
    f[0] = t[6]; f[1] = t[3]; f[2] = t[0];
    f[3] = t[7];              f[5] = t[1];
    f[6] = t[8]; f[7] = t[5]; f[8] = t[2];
}

void rotate_front(RubikCube *cube) {
    rotate_face(cube->front);
    char t6 = cube->up[6], t7 = cube->up[7], t8 = cube->up[8];
    cube->up[6] = cube->left[8]; cube->up[7] = cube->left[5]; cube->up[8] = cube->left[2];
    cube->left[8] = cube->down[2]; cube->left[5] = cube->down[1]; cube->left[2] = cube->down[0];
    cube->down[2] = cube->right[6]; cube->down[1] = cube->right[3]; cube->down[0] = cube->right[0];
    cube->right[6] = t8; cube->right[3] = t7; cube->right[0] = t6;
}

void rotate_back(RubikCube *cube) {
    rotate_face(cube->back);
    char t0 = cube->up[0], t1 = cube->up[1], t2 = cube->up[2];
    cube->up[0] = cube->right[2]; cube->up[1] = cube->right[5]; cube->up[2] = cube->right[8];
    cube->right[2] = cube->down[8]; cube->right[5] = cube->down[7]; cube->right[8] = cube->down[6];
    cube->down[8] = cube->left[0]; cube->down[7] = cube->left[3]; cube->down[6] = cube->left[6];
    cube->left[0] = t0; cube->left[3] = t1; cube->left[6] = t2;
}

void rotate_up(RubikCube *cube) {
    rotate_face(cube->up);
    char f0 = cube->front[0], f1 = cube->front[1], f2 = cube->front[2];
    cube->front[0] = cube->right[0]; cube->front[1] = cube->right[1]; cube->front[2] = cube->right[2];
    cube->right[0] = cube->back[0]; cube->right[1] = cube->back[1]; cube->right[2] = cube->back[2];
    cube->back[0] = cube->left[0]; cube->back[1] = cube->left[1]; cube->back[2] = cube->left[2];
    cube->left[0] = f0; cube->left[1] = f1; cube->left[2] = f2;
}

void rotate_down(RubikCube *cube) {
    rotate_face(cube->down);
    char f6 = cube->front[6], f7 = cube->front[7], f8 = cube->front[8];
    cube->front[6] = cube->left[6]; cube->front[7] = cube->left[7]; cube->front[8] = cube->left[8];
    cube->left[6] = cube->back[6]; cube->left[7] = cube->back[7]; cube->left[8] = cube->back[8];
    cube->back[6] = cube->right[6]; cube->back[7] = cube->right[7]; cube->back[8] = cube->right[8];
    cube->right[6] = f6; cube->right[7] = f7; cube->right[8] = f8;
}

void rotate_left(RubikCube *cube) {
    rotate_face(cube->left);
    char u0 = cube->up[0], u3 = cube->up[3], u6 = cube->up[6];
    cube->up[0] = cube->back[8]; cube->up[3] = cube->back[5]; cube->up[6] = cube->back[2];
    cube->back[8] = cube->down[0]; cube->back[5] = cube->down[3]; cube->back[2] = cube->down[6];
    cube->down[0] = cube->front[0]; cube->down[3] = cube->front[3]; cube->down[6] = cube->front[6];
    cube->front[0] = u0; cube->front[3] = u3; cube->front[6] = u6;
}

void rotate_right(RubikCube *cube) {
    rotate_face(cube->right);
    char u2 = cube->up[2], u5 = cube->up[5], u8 = cube->up[8];
    cube->up[2] = cube->front[2]; cube->up[5] = cube->front[5]; cube->up[8] = cube->front[8];
    cube->front[2] = cube->down[2]; cube->front[5] = cube->down[5]; cube->front[8] = cube->down[8];
    cube->down[2] = cube->back[6]; cube->down[5] = cube->back[3]; cube->down[8] = cube->back[0];
    cube->back[6] = u2; cube->back[3] = u5; cube->back[0] = u8;
}

EXPORT void play_move(RubikCube *cube, const char *cmd) {
    char face = cmd[0];
    char suffix = cmd[1];
    int times = 1;
    if (suffix == '\'') times = 3;
    else if (suffix == '2') times = 2;
    
    void (*rot_func)(RubikCube*) = NULL;
    if (face == 'F') rot_func = rotate_front;
    else if (face == 'B') rot_func = rotate_back;
    else if (face == 'U') rot_func = rotate_up;
    else if (face == 'D') rot_func = rotate_down;
    else if (face == 'L') rot_func = rotate_left;
    else if (face == 'R') rot_func = rotate_right;
    
    if (rot_func != NULL) {
        int i;
        for (i = 0; i < times; i++) rot_func(cube);
    }
}

EXPORT int is_solved(RubikCube *cube) {
    char *faces[6] = {cube->up, cube->down, cube->front, cube->back, cube->left, cube->right};
    int f, i;
    for (f = 0; f < 6; f++) {
        for (i = 1; i < 9; i++) {
            if (faces[f][i] != faces[f][0]) return 0;
        }
    }
    return 1;
}