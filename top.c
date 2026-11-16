#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <pwd.h>

/* ---------- terminal colours / cursor helpers ---------- */
#define KNRM  "\033[0m"
#define KRED  "\033[31m"
#define KGRN  "\033[32m"
#define KBLU  "\033[34m"
#define KCYN  "\033[36m"
#define KWHT  "\033[37m"
#define KBLK  "\033[30m"
#define BGRN  "\033[0;42m"
#define BWHT  "\033[0;47m"
#define FBLD  "\033[1;31m"
#define FLIN  "\033[2;31m"
#define FBLN  "\033[5;31m"
#define RESET "\033[0m"

#define gotoxy(x, y)  printf("\033[%d;%dH", (y), (x))
#define moveup(n)     printf("\033[%dA", (n))
#define moveleft(n)   printf("\033[%dD", (n))

#define KB_PER_GB 1048576
#define BAR_CHAR  "\xE2\x96\x92"   /* U+2592 */
#define MAX_BAR   8

typedef long long ll;

static const int refresh_rate = 1;   /* seconds between refreshes */
static int num_cores;

struct process {
    int    pid;
    char   name[64];
    char   state[8];
    double cpu;
    ll     time;        /* running time in seconds */
    int    priority;
    int    nice;
    int    user_id;
    ll     res;         /* resident memory (kB) */
    ll     virt;        /* virtual memory (kB)  */
};

/* ---------- small helpers ---------- */

static int isNumber(const char *s)
{
    for (; *s; s++)
        if (*s < '0' || *s > '9')
            return 0;
    return 1;
}

static void printHMS(int h, int m, int s)
{
    printf("%02d:%02d:%02d", h, m, s);
}

/* number of cores: "siblings" line of /proc/cpuinfo */
static int getNumCores(void)
{
    int cores = 0;
    char line[256];
    FILE *f = fopen("/proc/cpuinfo", "r");
    if (f) {
        while (fgets(line, sizeof line, f)) {
            if (strncmp(line, "siblings", 8) == 0) {
                sscanf(line, "%*[^0123456789]%d", &cores);
                break;
            }
        }
        fclose(f);
    }
    if (cores <= 0)
        cores = (int)sysconf(_SC_NPROCESSORS_ONLN);
    return cores;
}

/* ---------- reading process info ---------- */

static void readMemory(struct process *p)
{
    char path[64], line[256], word[32];
    ll val;
    FILE *f;

    snprintf(path, sizeof path, "/proc/%d/status", p->pid);
    f = fopen(path, "r");
    if (!f)
        return;

    while (fgets(line, sizeof line, f)) {
        word[0] = '\0';
        val = 0;
        sscanf(line, "%31s %lld", word, &val);
        if (strcmp(word, "VmSize:") == 0)
            p->virt = val;
        else if (strcmp(word, "VmRSS:") == 0) {
            p->res = val;
            break;
        }
    }
    fclose(f);
}

/* fills p; returns 0 if the process vanished or could not be read */
static int readProcess(int pid, double uptime, struct process *p)
{
    char path[64], line[1024];
    FILE *f;

    p->pid = pid;
    p->cpu = -1;  p->time = -1;  p->user_id = -1;
    p->priority = 101;  p->nice = 21;
    p->res = -1;  p->virt = -1;

    /* /proc/<pid>/stat */
    snprintf(path, sizeof path, "/proc/%d/stat", pid);
    f = fopen(path, "r");
    if (!f)
        return 0;
    if (!fgets(line, sizeof line, f)) {
        fclose(f);
        return 0;
    }
    fclose(f);

    /* the name is "(...)" and may contain spaces, so use the last ')' */
    char *open  = strchr(line, '(');
    char *close = strrchr(line, ')');
    if (!open || !close)
        return 0;
    snprintf(p->name, sizeof p->name, "%.*s", (int)(close - open + 1), open);

    ll utime, stime, starttime;
    int n = sscanf(close + 1,
                   " %7s"                                      /* state           */
                   " %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s"  /* ppid .. cmajflt */
                   " %lld %lld"                                /* utime stime     */
                   " %*s %*s"                                  /* cutime cstime   */
                   " %d %d"                                    /* priority nice   */
                   " %*s %*s"                                  /* threads itreal  */
                   " %lld",                                    /* starttime       */
                   p->state, &utime, &stime,
                   &p->priority, &p->nice, &starttime);
    if (n < 6)
        return 0;

    /* cpu percentage and running time */
    double ticks   = (double)sysconf(_SC_CLK_TCK);
    double elapsed = uptime - starttime / ticks;
    double used    = (utime + stime) / ticks;
    p->cpu  = used * 100 / elapsed;
    p->time = (ll)elapsed;

    /* user id */
    snprintf(path, sizeof path, "/proc/%d/loginuid", pid);
    f = fopen(path, "r");
    if (f) {
        if (fscanf(f, "%d", &p->user_id) != 1)
            p->user_id = -1;
        fclose(f);
    }

    readMemory(p);
    return 1;
}

/* ---------- printing a process row ---------- */

/* prints a kB value as MB / G / T; 'colored' adds the colour changes */
static void printSize(ll kb, int colored)
{
    float mb = kb / (float)1024;

    if (mb < 1024) {
        printf("%.2f\t", mb);
    } else if (mb / 1024.0 < 1024) {
        if (colored) printf("%s", KWHT);
        printf("%.2fG\t", mb / 1024);
        if (colored) printf("%s", KCYN);
    } else {
        if (colored) printf("%s", KRED);
        printf("%.2fT\t", mb / (1024 * 1024));
        if (colored) printf("%s", KCYN);
    }
}

static void displayProcess(const struct process *p)
{
    if (p->virt < 0 || p->res < 0)
        return;

    /* PID */
    printf("%s%d\t", KWHT, p->pid);

    /* USER (name if known, otherwise the number) */
    struct passwd *pw = (p->user_id >= 0) ? getpwuid(p->user_id) : NULL;
    if (pw)
        printf("%s%s\t", KBLU, pw->pw_name);
    else
        printf("%s%d\t", KBLU, p->user_id);

    /* PRI, NI */
    printf("%s%d\t", p->priority == 20 ? KWHT : KCYN, p->priority);
    printf("%s%d\t", p->nice < 0 ? KRED : KNRM, p->nice);

    /* VIRT, RES */
    printf("%s", KCYN);
    printSize(p->virt, 1);
    printSize(p->res, 0);

    /* STATE, CPU% */
    printf("%s%s     ", KWHT, p->state);
    printf("%s%.2f\t", KWHT, p->cpu);

    /* TIME+ */
    int h = (int)(p->time / 3600);
    int m = (int)(p->time / 60) - 60 * h;
    int s = (int)(p->time - 60 * m - 3600 * h);
    printf("%s%s", KWHT, FBLD);
    printHMS(h, m, s);
    printf("\t");

    /* PROGRAM */
    printf("%s%s%s\n", KGRN, p->name, KNRM);
}

/* ---------- CPU bars (top-right corner) ---------- */

/* overall usage (index 0) followed by one value per core */
static float *getCPUusage(void)
{
    FILE *f = fopen("/proc/stat", "r");
    if (!f)
        return NULL;

    float *usage = calloc(num_cores + 1, sizeof(float));
    char line[256], label[16];

    for (int i = 0; i <= num_cores; i++) {
        ll v[7] = {0};
        if (!fgets(line, sizeof line, f))
            break;
        sscanf(line, "%15s %lld %lld %lld %lld %lld %lld %lld",
               label, &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6]);

        ll total = 0;
        for (int j = 0; j < 7; j++)
            total += v[j];

        float idle_pct = (v[3] * 100) / (float)total;
        usage[i] = 100 - idle_pct;
    }
    fclose(f);
    return usage;
}

/* draws one vertical bar whose label sits at screen column x, row 12 */
static void drawBar(int x, const char *label, int label_len, float usage,
                    float max, const char *pct_color, const char *bar_color)
{
    gotoxy(x, 12);
    printf("%s", label);
    moveup(1);
    moveleft(label_len);
    printf("%s%.1f%%%s", pct_color, usage, KNRM);
    moveup(1);
    moveleft(4);

    printf("%s", bar_color);
    for (int i = 1; i <= MAX_BAR * (usage / max); i++) {
        printf(BAR_CHAR);
        moveup(1);
        moveleft(1);
    }
    printf("%s", KNRM);
}

static void displayCPU(void)
{
    float *usage = getCPUusage();
    if (!usage)
        return;

    float max = 0;
    for (int i = 0; i <= num_cores; i++)
        if (usage[i] > max)
            max = usage[i];

    int x = 88;
    drawBar(x, "AVG", 3, usage[0], max, KCYN, KGRN);

    for (int i = 1; i <= num_cores; i++) {
        char label[32];
        snprintf(label, sizeof label, "core%d", i);
        x -= 6;
        drawBar(x, label, 5, usage[i], max, KBLU,
                usage[i] == max ? KRED : KBLU);
    }
    gotoxy(1, 1);
    free(usage);
}

/* ---------- header section ---------- */

static void printUptimeAndIdle(double *uptime_out)
{
    float up = 0, idle = 0;
    FILE *f = fopen("/proc/uptime", "r");
    if (!f)
        return;
    fscanf(f, "%f %f", &up, &idle);
    fclose(f);
    *uptime_out = up;

    int h = (int)(up / (60 * 60));
    int m = (int)(up / 60) - 60 * h;
    int s = (int)(up - 60 * m - 60 * 60 * h);
    int idle_pct = (int)(((idle / num_cores) / up) * 100);

    printf("UpTime \t\t: %s%s", FBLD, KBLU);
    printHMS(h, m, s);
    printf("\n%s", RESET);

    const char *color = idle_pct > 80 ? KGRN : idle_pct > 20 ? KWHT : KRED;
    printf("Idle \t\t: %s%s%d%%\n%s", FBLD, color, idle_pct, RESET);
}

static void printMemory(void)
{
    ll total = 0, avail = 0, swap_total = 0, swap_free = 0;
    char line[256], word[32];
    ll val;
    FILE *f = fopen("/proc/meminfo", "r");
    if (!f) {
        printf("could not open file /proc/meminfo\n");
        return;
    }
    while (fgets(line, sizeof line, f)) {
        word[0] = '\0';
        val = 0;
        sscanf(line, "%31s %lld", word, &val);
        if      (strcmp(word, "MemTotal:") == 0)     total = val;
        else if (strcmp(word, "MemAvailable:") == 0) avail = val;
        else if (strcmp(word, "SwapTotal:") == 0)    swap_total = val;
        else if (strcmp(word, "SwapFree:") == 0) { swap_free = val; break; }
    }
    fclose(f);

    printf("Memory used \t: %.2f/%.2f GB\n",
           (float)(total - avail) / KB_PER_GB, (float)total / KB_PER_GB);
    printf("Swap used   \t: %.2f/%.2f GB\n",
           (float)(swap_total - swap_free) / KB_PER_GB, (float)swap_total / KB_PER_GB);
}

static void printBattery(void)
{
    int  percentage = -1;
    char status[20] = "", level[20] = "";
    FILE *f;

    if ((f = fopen("/sys/class/power_supply/BAT1/capacity", "r"))) {
        fscanf(f, "%d", &percentage);
        fclose(f);
    }
    if ((f = fopen("/sys/class/power_supply/BAT1/status", "r"))) {
        fscanf(f, "%19s", status);
        fclose(f);
    }
    if ((f = fopen("/sys/class/power_supply/BAT1/capacity_level", "r"))) {
        fscanf(f, "%19s", level);
        fclose(f);
    }

    printf("Bat\t\t:");
    if (percentage < 20 && strcmp(status, "Discharging") == 0)
        printf("%s%s", KRED, FBLN);
    printf(" %s - %d%% \n\t\t  (%s)\n", status, percentage, level);
    printf(RESET);
}

/* ---------- one full screen refresh ---------- */

static int drawFrame(void)
{
    system("clear");
    displayCPU();

    printf("%s%s%s", FLIN, BWHT, KBLK);
    printf("--------------------------------------------TOP PROGRAM--------------------------------------------\n\n\n");
    printf(RESET);

    double uptime = 0;
    printUptimeAndIdle(&uptime);
    printf("Cores \t\t: %d\n", num_cores);
    printMemory();
    printBattery();
    printf("\n\n");

    printf("%s%s", BGRN, KBLK);
    printf("PID     USER    PRI     NI      VIR(mb) RES(mb) STATE CPU%%      TIME+           PROGRAM            \n");
    printf(RESET);

    DIR *dir = opendir("/proc");
    if (!dir) {
        printf("Error opening /proc\n");
        return 0;
    }
    struct dirent *de;
    while ((de = readdir(dir)) != NULL) {
        if (!isNumber(de->d_name))
            continue;
        struct process p;
        if (readProcess(atoi(de->d_name), uptime, &p))
            displayProcess(&p);
    }
    closedir(dir);
    return 1;
}

int main(void)
{
    num_cores = getNumCores();

    while (1) {
        if (!drawFrame())
            return 0;
        fflush(stdout);
        sleep(refresh_rate);
    }
}