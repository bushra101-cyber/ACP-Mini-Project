#include <stdio.h>
#include <stdlib.h>
#define ROWS 25
#define COLS 60
#define MAX_OBJECTS 100

enum ShapeType { LINE = 1, RECTANGLE, TRIANGLE, CIRCLE };

typedef struct {
    int id;
    int type;
    int active;
    int x1, y1, x2, y2, x3, y3, r;
} Object;

char canvas[ROWS][COLS];
Object objects[MAX_OBJECTS];
int objectCount = 0;
int nextId = 1;

int absVal(int n) {
    return (n < 0) ? -n : n;
}

void clearCanvas() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            canvas[i][j] = '_';
        }
    }
}

void plotPoint(int x, int y) {
    if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {
        canvas[y][x] = '*';
    }
}

void displayCanvas() {
    printf("\n   ");
    for (int j = 0; j < COLS; j++) {
        printf("%d", j % 10);
    }
    printf("\n");

    for (int i = 0; i < ROWS; i++) {
        printf("%2d ", i);
        for (int j = 0; j < COLS; j++) {
            printf("%c", canvas[i][j]);
        }
        printf("\n");
    }
}

void drawLine(int x1, int y1, int x2, int y2) {
    int dx = absVal(x2 - x1);
    int dy = absVal(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        plotPoint(x1, y1);
        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void drawRectangle(int x1, int y1, int x2, int y2) {
    drawLine(x1, y1, x2, y1);
    drawLine(x2, y1, x2, y2);
    drawLine(x2, y2, x1, y2);
    drawLine(x1, y2, x1, y1);
}

void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3) {
    drawLine(x1, y1, x2, y2);
    drawLine(x2, y2, x3, y3);
    drawLine(x3, y3, x1, y1);
}

void plotCirclePoints(int xc, int yc, int x, int y) {
    plotPoint(xc + x, yc + y);
    plotPoint(xc - x, yc + y);
    plotPoint(xc + x, yc - y);
    plotPoint(xc - x, yc - y);
    plotPoint(xc + y, yc + x);
    plotPoint(xc - y, yc + x);
    plotPoint(xc + y, yc - x);
    plotPoint(xc - y, yc - x);
}

void drawCircle(int xc, int yc, int r) {
    int x = 0;
    int y = r;
    int p = 1 - r;

    plotCirclePoints(xc, yc, x, y);

    while (x < y) {
        x++;
        if (p < 0) {
            p = p + 2 * x + 1;
        } else {
            y--;
            p = p + 2 * (x - y) + 1;
        }
        plotCirclePoints(xc, yc, x, y);
    }
}

void redrawCanvas() {
    clearCanvas();

    for (int i = 0; i < objectCount; i++) {
        if (!objects[i].active) continue;

        switch (objects[i].type) {
            case LINE:
                drawLine(objects[i].x1, objects[i].y1, objects[i].x2, objects[i].y2);
                break;

            case RECTANGLE:
                drawRectangle(objects[i].x1, objects[i].y1, objects[i].x2, objects[i].y2);
                break;

            case TRIANGLE:
                drawTriangle(objects[i].x1, objects[i].y1,
                             objects[i].x2, objects[i].y2,
                             objects[i].x3, objects[i].y3);
                break;

            case CIRCLE:
                drawCircle(objects[i].x1, objects[i].y1, objects[i].r);
                break;
        }
    }
}

void addObject() {
    if (objectCount >= MAX_OBJECTS) {
        printf("Object list is full.\n");
        return;
    }

    Object obj;
    obj.id = nextId++;
    obj.active = 1;

    printf("\nChoose object type:\n");
    printf("1. Line\n2. Rectangle\n3. Triangle\n4. Circle\n");
    printf("Enter choice: ");
    scanf("%d", &obj.type);

    switch (obj.type) {
        case LINE:
            printf("Enter x1 y1 x2 y2: ");
            scanf("%d%d%d%d", &obj.x1, &obj.y1, &obj.x2, &obj.y2);
            break;

        case RECTANGLE:
            printf("Enter top-left x1 y1 and bottom-right x2 y2: ");
            scanf("%d%d%d%d", &obj.x1, &obj.y1, &obj.x2, &obj.y2);
            break;

        case TRIANGLE:
            printf("Enter x1 y1 x2 y2 x3 y3: ");
            scanf("%d%d%d%d%d%d", &obj.x1, &obj.y1, &obj.x2, &obj.y2, &obj.x3, &obj.y3);
            break;

        case CIRCLE:
            printf("Enter center x y and radius: ");
            scanf("%d%d%d", &obj.x1, &obj.y1, &obj.r);
            break;

        default:
            printf("Invalid shape type.\n");
            return;
    }

    objects[objectCount++] = obj;
    redrawCanvas();
    printf("Object added with ID = %d\n", obj.id);
}

int findObjectIndexById(int id) {
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].id == id && objects[i].active) {
            return i;
        }
    }
    return -1;
}

void deleteObject() {
    int id;
    printf("Enter object ID to delete: ");
    scanf("%d", &id);

    int index = findObjectIndexById(id);
    if (index == -1) {
        printf("Object not found.\n");
        return;
    }

    objects[index].active = 0;
    redrawCanvas();
    printf("Object deleted.\n");
}

void modifyObject() {
    int id;
    printf("Enter object ID to modify: ");
    scanf("%d", &id);

    int index = findObjectIndexById(id);
    if (index == -1) {
        printf("Object not found.\n");
        return;
    }

    Object *obj = &objects[index];

    printf("Modifying object ID %d\n", obj->id);

    switch (obj->type) {
        case LINE:
            printf("Enter new x1 y1 x2 y2: ");
            scanf("%d%d%d%d", &obj->x1, &obj->y1, &obj->x2, &obj->y2);
            break;

        case RECTANGLE:
            printf("Enter new top-left x1 y1 and bottom-right x2 y2: ");
            scanf("%d%d%d%d", &obj->x1, &obj->y1, &obj->x2, &obj->y2);
            break;

        case TRIANGLE:
            printf("Enter new x1 y1 x2 y2 x3 y3: ");
            scanf("%d%d%d%d%d%d", &obj->x1, &obj->y1, &obj->x2, &obj->y2, &obj->x3, &obj->y3);
            break;

        case CIRCLE:
            printf("Enter new center x y and radius: ");
            scanf("%d%d%d", &obj->x1, &obj->y1, &obj->r);
            break;
    }

    redrawCanvas();
    printf("Object modified.\n");
}

void listObjects() {
    printf("\nActive Objects:\n");
    for (int i = 0; i < objectCount; i++) {
        if (!objects[i].active) continue;

        printf("ID=%d ", objects[i].id);
        switch (objects[i].type) {
            case LINE:
                printf("LINE (%d,%d) to (%d,%d)\n",
                       objects[i].x1, objects[i].y1, objects[i].x2, objects[i].y2);
                break;

            case RECTANGLE:
                printf("RECTANGLE (%d,%d) to (%d,%d)\n",
                       objects[i].x1, objects[i].y1, objects[i].x2, objects[i].y2);
                break;

            case TRIANGLE:
                printf("TRIANGLE (%d,%d), (%d,%d), (%d,%d)\n",
                       objects[i].x1, objects[i].y1,
                       objects[i].x2, objects[i].y2,
                       objects[i].x3, objects[i].y3);
                break;

            case CIRCLE:
                printf("CIRCLE center (%d,%d), r=%d\n",
                       objects[i].x1, objects[i].y1, objects[i].r);
                break;
        }
    }
}

int main() {
    int choice;

    clearCanvas();

    while (1) {
        printf("\n===== 2D GRAPHICS EDITOR =====\n");
        printf("1. Add object\n");
        printf("2. Delete object\n");
        printf("3. Modify object\n");
        printf("4. Display picture\n");
        printf("5. List objects\n");
        printf("6. Clear picture\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addObject();
                break;

            case 2:
                deleteObject();
                break;

            case 3:
                modifyObject();
                break;

            case 4:
                displayCanvas();
                break;

            case 5:
                listObjects();
                break;

            case 6:
                objectCount = 0;
                nextId = 1;
                clearCanvas();
                printf("Picture cleared.\n");
                break;

            case 7:
                printf("Exiting program.\n");
                return 0;

            default:
                printf("Invalid choice.\n");
        }
    }

    return 0;
}