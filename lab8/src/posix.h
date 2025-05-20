#define MAX_THREADS 64

typedef struct {
    int start_row;
    int end_row;
    char *foreground;
    char *background;
    int width;
    int height;
} ThreadDataTypeDef;

