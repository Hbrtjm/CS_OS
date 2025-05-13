#define MAX_THREADS 64

typedef struct {
    int id;
    int start_row;
    int end_row;
    char *foreground;
    char *background;
    int width;
    int height;
} ThreadDataTypeDef;

