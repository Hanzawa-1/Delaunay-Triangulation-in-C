#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

// Constants
#define max_ele 100
#define max_iter 100
#define PI 3.141592654
#define max_x 0.9
#define max_y 0.9
#define min_x 0.1
#define min_y 0.1
#define tolerance 0.0001
#define MAX_ERROR 1000000

// Function to check if three points are collinear
int collinear(float x1, float y1, float x2, float y2, float x3, float y3) {
    float a = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);
    return a < tolerance ? 1 : 0;
}

// Function to check if an element is unique in the Delaunay elements array
int isUniqueElement(int delone_elements[][3], int a, int b, int c, int count_delone_ele) {
    for (int i = 0; i < count_delone_ele; i++) {
        if (delone_elements[i][0] == a && delone_elements[i][1] == b && delone_elements[i][2] == c) {
            return 0;
        }
    }
    return 1;
}

// Function to sort a 2D array row-wise
int **sortRowWise(int m[][3], int r, int c) {
    int t = 0;
    // Bubble sort each row
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            for (int k = 0; k < c - j - 1; k++) {
                if (m[i][k] > m[i][k + 1]) {
                    t = m[i][k];
                    m[i][k] = m[i][k + 1];
                    m[i][k + 1] = t;
                }
            }
        }
    }

    // Allocate memory for sorted rows
    int **sortedRows = malloc(sizeof(int *) * r);
    for (int i = 0; i < r; i++)
        sortedRows[i] = malloc(sizeof(int) * c);

    // Copy sorted data to the new array
    for (int i = 0; i < r; i++)
        for (int j = 0; j < c; j++)
            sortedRows[i][j] = m[i][j];

    return sortedRows;
}

// Function to find the circumcircle of a triangle
void findCircle(float x1, float y1, float x2, float y2, float x3, float y3, float *xc, float *yc, float *R) {
    float x12 = x1 - x2;
    float x13 = x1 - x3;
    float y12 = y1 - y2;
    float y13 = y1 - y3;
    float y31 = y3 - y1;
    float y21 = y2 - y1;
    float x31 = x3 - x1;
    float x21 = x2 - x1;

    float sx13 = pow(x1, 2) - pow(x3, 2);
    float sy13 = pow(y1, 2) - pow(y3, 2);
    float sx21 = pow(x2, 2) - pow(x1, 2);
    float sy21 = pow(y2, 2) - pow(y1, 2);

    float f = (sx13 * x12 + sy13 * x12 + sx21 * x13 + sy21 * x13) / (2 * (y31 * x12 - y21 * x13));
    float g = (sx13 * y12 + sy13 * y12 + sx21 * y13 + sy21 * y13) / (2 * (x31 * y12 - x21 * y13));
    float c = -pow(x1, 2) - pow(y1, 2) - 2 * g * x1 - 2 * f * y1;

    // Center of the circle
    float h = -g;
    float k = -f;
    float sqr_of_r = h * h + k * k - c;

    // Radius of the circle
    float r = sqrt(sqr_of_r);

    *xc = h;
    *yc = k;
    *R = r;
}

// Function to check if a point lies inside a given circle
void isInsideCircle(float xc, float yc, float r, int *result, float x, float y) {
    float temp = (x - xc) * (x - xc) + (y - yc) * (y - yc) - r * r;
    *result = temp > 0 ? 0 : 1;
}

// Function to check if any node lies inside the circumcircle of a given triangle
int anyNodeLiesInside(float nodes[][2], int numNodes, int n1, int n2, int n3) {
    int count = 0;
    for (int i = 0; i < numNodes; i++) {
        if ((i != n1) && (i != n2) && (i != n3)) {
            float xc = 0, yc = 0, r = 0;
            findCircle(nodes[n1][0], nodes[n1][1], nodes[n2][0], nodes[n2][1], nodes[n3][0], nodes[n3][1], &xc, &yc, &r);
            int ans_circle = 0;
            isInsideCircle(xc, yc, r, &ans_circle, nodes[i][0], nodes[i][1]);
            if (ans_circle == 1) {
                count++;
                break;
            }
        }
    }
    return count > 0 ? 1 : 0;
}

// Function to reorder the nodes in anticlockwise direction
int **anticlockwiseNodes(int delone_elements[][3], int count_delone_ele, float nodes[][2]) {
    int reorder[count_delone_ele][3];

    for (int gg = 0; gg < count_delone_ele; gg++) {
        float xc = 0, yc = 0, r = 0;
        int n1 = delone_elements[gg][0];
        int n2 = delone_elements[gg][1];
        int n3 = delone_elements[gg][2];

        findCircle(nodes[n1][0], nodes[n1][1], nodes[n2][0], nodes[n2][1], nodes[n3][0], nodes[n3][1], &xc, &yc, &r);

        float ang_n1 = atan2(nodes[n1][1] - yc, nodes[n1][0] - xc);
        float ang_n2 = atan2(nodes[n2][1] - yc, nodes[n2][0] - xc);
        float ang_n3 = atan2(nodes[n3][1] - yc, nodes[n3][0] - xc);

        float ele_angles[3][2] = { {n1, ang_n1}, {n2, ang_n2}, {n3, ang_n3} };

        for (int j = 0; j < 3; j++) {
            if (ele_angles[j][1] < 0) {
                ele_angles[j][1] = 2 * PI + ele_angles[j][1];
            }
        }

        float angles[] = {ele_angles[0][1], ele_angles[1][1], ele_angles[2][1]};
        int length = 3;

        // Sort angles in ascending order
        for (int i = 0; i < length; i++) {
            for (int j = i + 1; j < length; j++) {
                if (angles[i] > angles[j]) {
                    float temp = angles[i];
                    angles[i] = angles[j];
                    angles[j] = temp;
                }
            }
        }

        // Reorder nodes based on sorted angles
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                if (angles[i] == ele_angles[j][1]) {
                    reorder[gg][i] = ele_angles[j][0];
                }
            }
        }
    }

    // Allocate memory for sorted rows
    int **sortedRows = malloc(sizeof(int *) * count_delone_ele);
    for (int i = 0; i < count_delone_ele; i++)
        sortedRows[i] = malloc(sizeof(int) * 3);

    // Copy reordered nodes to the new array
    for (int i = 0; i < count_delone_ele; i++)
        for (int j = 0; j < 3; j++)
            sortedRows[i][j] = reorder[i][j];

    return sortedRows;
}

int main() {
    // Input nodes
    float nodes[][2] = { { 0.0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, {.3, .8}, {.7, .2} };
    
    // Delaunay nodes array
    int delone_nodes[max_ele][3];
    int delone_count = 0;

    // Number of nodes
    int numNodes = sizeof(nodes) / sizeof(nodes[0]);
    printf("\nNumber of nodes: %d\n", numNodes);

    // Loop through each combination of nodes to form Delaunay triangles
    for (int n1 = 0; n1 < numNodes; n1++) {
        for (int n2 = 0; n2 < numNodes; n2++) {
            for (int n3 = 0; n3 < numNodes; n3++) {
                int ans_colinear = collinear(nodes[n1][0], nodes[n1][1], nodes[n2][0], nodes[n2][1], nodes[n3][0], nodes[n3][1]);
                if ((n1 != n2) && (n1 != n3) && (n2 != n3) && (ans_colinear != 1)) {
                    int ans = anyNodeLiesInside(nodes, numNodes, n1, n2, n3);
                    if (ans == 0) {
                        delone_nodes[delone_count][0] = n1;
                        delone_nodes[delone_count][1] = n2;
                        delone_nodes[delone_count][2] = n3;
                        delone_count++;
                    }
                }
            }
        }
    }

    // Sort Delaunay nodes row-wise
    int **sortedRows = sortRowWise(delone_nodes, delone_count, 3);

    // Find unique Delaunay elements
    int delone_elements[delone_count][3];
    int count_delone_ele = 1;
    delone_elements[0][0] = sortedRows[0][0];
    delone_elements[0][1] = sortedRows[0][1];
    delone_elements[0][2] = sortedRows[0][2];

    for (int i = 1; i < delone_count; i++) {
        int ans = isUniqueElement(delone_elements, sortedRows[i][0], sortedRows[i][1], sortedRows[i][2], count_delone_ele);
        if (ans == 1) {
            delone_elements[count_delone_ele][0] = sortedRows[i][0];
            delone_elements[count_delone_ele][1] = sortedRows[i][1];
            delone_elements[count_delone_ele][2] = sortedRows[i][2];
            count_delone_ele++;
        }
    }

    // Print unique Delaunay triangles
    for (int i = 0; i < count_delone_ele; i++)
        printf("\nUnique Delaunay triangles are [%d] {%d, %d, %d}\n", i + 1, delone_elements[i][0] + 1, delone_elements[i][1] + 1, delone_elements[i][2] + 1);

    // Reorder nodes in anticlockwise direction
    int **conectivity = anticlockwiseNodes(delone_elements, count_delone_ele, nodes);

    // Print directionalized Delaunay triangles
    for (int i = 0; i < count_delone_ele; i++)
        printf("\nDirectionalized Delaunay triangles are [%d] {%d, %d, %d}\n", i + 1, conectivity[i][0] + 1, conectivity[i][1] + 1, conectivity[i][2] + 1);

    return 0;
}
