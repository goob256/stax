#define TIMER_FREQ  1000

/* these are missing in DOS, and security isn't an issue :) */
#ifdef DJGPP
#define snprintf(s, n, fmt, args...)     sprintf(s, fmt, ## args)
#define vsnprintf(s, n, fmt, ap)         vsprintf(s, fmt, ap)
#endif

#define MIDI_VOLUME 128

#define BLOCK_SIZE  10
#define PANEL_W     14
#define PANEL_H     16
#define NUM_BLOCKS  4
#define RAND_BLOCK  (1+(int)((double)NUM_BLOCKS*rand()/(RAND_MAX+1.0)))

#define GAME_STYLE_SUCKA    0
#define GAME_STYLE_CLOBBER  1
#define GAME_STYLE_SHIFTY   2
#define GAME_STYLE_COOP     3
#define GAME_STYLE_PUZZLE   4
#define NUM_GAME_STYLES     5

#define FALL_SPEED       0.05
#define RISE_SPEED       0.0005
#define POP_SPEED        0.001
#define POP_DURATION     (TIMER_FREQ)    /* one second */
#define FLASH_DURATION   (TIMER_FREQ/8)
#define BACK_DURATION    (TIMER_FREQ)
#define PLAYER_DURATION  (TIMER_FREQ*2)

/* colors */
#define C_DGREEN  2
#define C_CYAN    3
#define C_RED     4
#define C_PURPLE  5
#define C_DYELLOW 6
#define C_GREY    7
#define C_DGREY   8
#define C_BCYAN   11
#define C_BPURPLE 13
#define C_WHITE   15
#define C_YELLOW  71
#define C_GREEN   96
#define C_BLACK   224

typedef struct {
	int style;
	char theme[256];
	int input1;
	int input2;
	int sound;
} Config;

typedef struct Block {
	int x;
	fixed y;
	int type;
	int popping;
	int glued;
	int data;                    /* games can use this for whatever the want */
	struct Block *next, *prev;
} Block;

typedef struct {
	BITMAP *bmp;                 /* sub bitmap of virtual screen */
	int x, y;
	fixed fall_speed;
	fixed rise_speed;
	Block *blocks[PANEL_W];
	int blocks_popping;
	int pop_count;
	int flash_count;            /* for timing block flashing */
	int type_flashing;
	int player_bitmap;          /* eg, idle, win, .. */
	int player_count;           /* for timing the above bitmap */
	int player_number;          /* 0=player 1, 1=player 2 */
} Panel;

#define INPUT_KB1     0
#define INPUT_KB2     1
#define INPUT_JOY1    2
#define INPUT_JOY2    3

typedef struct {
	int left;
	int right;
	int up;
	int down;
	int button;
} Input;

#define PLAYER_IDLE          0
#define PLAYER_COMBO         1
#define PLAYER_WIN           2
#define PLAYER_LOSE          3
#define NUM_PLAYER_ACTIONS   4

#define MAX_BACKGROUND_FRAMES  100

typedef struct {
	DATAFILE *datafile;
	BITMAP *preview;
	BITMAP *background[MAX_BACKGROUND_FRAMES];
	BITMAP *blocks[NUM_BLOCKS];
	BITMAP *blocks_h[NUM_BLOCKS];
	BITMAP *player_bitmaps[2][NUM_PLAYER_ACTIONS];
	SAMPLE *player_samples[2][NUM_PLAYER_ACTIONS];
	SAMPLE *start;
	SAMPLE *collide;
	SAMPLE *pop;
	int background_frame;
	int background_count;
} Theme;


/* config.c */
void read_config(void);
void config_loop(void);
int toggle_proc(int, DIALOG *, int);
int shadow_text_proc(int, DIALOG *, int);
int text_button_proc(int, DIALOG *, int);

/* input.c */
void get_input(Input *, int, int);

/* main.c */
extern int tick;
extern BITMAP *vs;
extern Config cfg;
extern DATAFILE *dat;
extern char *argv0;
extern RGB palette[PAL_SIZE];

/* shared.c */
char *datapath(char *);
void *find_object_data(DATAFILE *, char *, ...);
void textprintf_shadow(BITMAP *, FONT *, int, int, int, int, char *, ...);
void error(char *, char *, char *);
int prompt(char *, char *, char *);
void screenshot(void);
int game_over(Panel *, Panel *, Theme *);
void do_pause(void);
void zoom_image(BITMAP *);
void focus_image(BITMAP *);
int block_add(Block *hash[PANEL_W], int, fixed, int);
void block_delete(Block *hash[PANEL_W], Block *);
int move_blocks(Panel *, Theme *, int);
int check_blocks(Panel *);
void draw_panel(Panel *, Theme *);
void draw_background(Panel *, Panel *, Theme *, int);
void free_panel(Panel *);
Panel *create_panel(int, int, int, int, int);
void free_theme(Theme *);
Theme *load_theme(char *);

/* game loops */
void sucka_loop(void);
void clobber_loop(void);
void shifty_loop(void);
void puzzle_loop(void);
void coop_loop(void);
