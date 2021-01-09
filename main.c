#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define LINELEN 1000
#define MAXACTS 10000
#define MAXLINES 10000
#define putd(x) printf(#x ": %d\n", x)

char helptext[] = "date format yyyy_mm_dd or yyyy-mm-dd\n\
command format <mode> [date1] [date2] [flags]\n\
time format hhmm\n\
commands:\n\
(h)elp      help\n\
(s)um       summarize activity between two dates\n\
s(p)ec      summarize certain activity between two dates\n\
(c)al       summarize activity by day\n\
(q)uit      quit, duh";

char wdays[][50] = {"sun", "mon", "tue", "wed", "thu", "fri", "sat"};
enum mode{SPEC, SUM, CAL, HELP, ERR, QUIT};

typedef struct activity
{
    int mins;
    char *name;
} activity;

char *insts[MAXACTS];
int insttimes[MAXACTS];
int numinsts = 0;

struct tm *today;
//int curdate[3];
struct tm curdate;
struct tm oldcurdate;

activity acts[MAXACTS];
int numacts = 0;

enum mod{SET, ADD};

int streq(char *c1, char *c2)
{
    return strcmp(c1, c2) == 0;
}

void modact(char *name, enum mod how, int amt)
{
    for(int i = 0; i < numacts; i++)
    {
        if(streq(acts[i].name, name))
        {
            if(how == SET)
            {
                acts[i].mins = amt;
            }
            else if(how == ADD)
            {
                acts[i].mins += amt;
            }
            return;
        }
    }
    acts[numacts].name = name;
    acts[numacts].mins = amt;
    numacts++;
}

void addinst(char *name, int time)
{
    for(int i = 0; i < numinsts; i++)
    {
        if(strcmp(insts[i], name) == 0)
            return;
    }
    insts[numinsts] = name;
    insttimes[numinsts] = time;
    numinsts++;
}

int trydate(char *line)
{
    oldcurdate = curdate;
    int tempyear, tempmonth, tempday;
    if(sscanf(line, "%d%*[_-]%d%*[_-]%d", &tempyear, &tempmonth, &tempday) == 3)
    {
        curdate.tm_year = tempyear-1900, curdate.tm_mon = tempmonth-1, curdate.tm_mday = tempday;
        return 1;
    }
    return 0;
}

int tryrange(char *line, int detail)
{
    int comp1, comp2;
    int h1,m1, h2,m2;
    struct tm time1, time2;
    time_t objtime1, objtime2;

    char *name = malloc(LINELEN);
    int works = 0;
    if(detail)
        works = (sscanf(line, "%d%*[_-]%d %[^#]", &comp1, &comp2, name) == 3);
    else
        works = (sscanf(line, "%d%*[_-]%d %s", &comp1, &comp2, name) == 3);
    if(works)
    {
        h1 = comp1/100, m1 = comp1 % 100;
        h2 = comp2/100, m2 = comp2 % 100;
        time1 = (struct tm) {.tm_min = m1, .tm_hour = h1, .tm_mday = curdate.tm_mday, .tm_mon = curdate.tm_mon, .tm_year = curdate.tm_year};
        time2 = (struct tm) {.tm_min = m2, .tm_hour = h2, .tm_mday = curdate.tm_mday, .tm_mon = curdate.tm_mon, .tm_year = curdate.tm_year};

        if(comp2 < comp1)
        {
            time2.tm_mday +=1;
        }
        objtime1 = mktime(&time1), objtime2 = mktime(&time2);
        int diff = (int) difftime(objtime2, objtime1)/60;

        modact(name, ADD, diff);
        return 1;
    }

    return 0;
}

int tryinst(char *line)
{
    //int comp;
    //int h, m;
    //struct tm time;
    //time_t objtime;

    //char *name = malloc(LINELEN);
    //if(sscanf(line, "%d %s", &comp, name) == 2)
    //{
    //    h = comp/100, m = comp % 100;
    //    time = (struct tm) {.tm_min = m, .tm_hour = h, .tm_mday = curdate.tm_mday, .tm_mon = curdate.tm_mon-1, .tm_year = curdate.tm_year-1900};
    //    objtime
    //}
    int time;
    char *name = malloc(LINELEN);
    if(sscanf(line, "%d %s", &time, name) == 2)
    {
        addinst(name, time);
    }
}

void printsum()
{
    if(numacts == 0 && numinsts == 0)
    {
        putchar('\n');
        return;
    }

    int sum = 0;
    for(int i = 0; i < numacts; i++)
    {
        printf("\t%-20s %02d:%02d\n", acts[i].name, acts[i].mins/60, acts[i].mins%60);
        sum += acts[i].mins;
    }
    printf("\t%-20s %02d:%02d\n", "SUM TOTAL", sum/60, sum%60);

    putchar('\n');
    if(numinsts == 0) return;
    for(int i = 0; i < numinsts; i++)
    {
        printf("\t%-20s %02d:%02d\n", insts[i], insttimes[i]/100, insttimes[i]%100);
    }
    putchar('\n');
}

void freeprev()
{
    int i;
    for(i = 0; i < numinsts; i++)
    {
        free(insts[i]);
    }
    for(i = 0; i < numacts; i++)
    {
        free(acts[i].name);
    }
    numinsts = numacts = 0;
}

int datecmp(struct tm t1, struct tm t2)
{
    if(t1.tm_year == t2.tm_year)
    {
        if(t1.tm_mon == t2.tm_mon)
            return t1.tm_mday - t2.tm_mday;
        else return t1.tm_mon - t2.tm_mon;
    }
    else return t1.tm_year - t2.tm_year;
}

void sumdate()
{
    void update(struct tm *timestruct);
    update(&curdate);
    printf("%04d-%02d-%02d", curdate.tm_year+1900, curdate.tm_mon+1, curdate.tm_mday);
    printf(",%s\n", wdays[curdate.tm_wday]);
    printsum();
}

enum mode choosemode(char *in)
{
    if(streq(in, "sum"))
        return SUM;
    if(streq(in, "spec"))
        return SPEC;
    if(streq(in, "cal"))
        return CAL;
    if(streq(in, "help"))
        return HELP;
    if(streq(in, "quit"))
        return QUIT;

    if(strlen(in) > 1)
        return ERR;

    switch(in[0])
    {
        case 's':
            return SUM;
        case 'p':
            return SPEC;
        case 'c':
            return CAL;
        case 'h':
            return HELP;
        case 'q': return QUIT;
    }

    return ERR;
}

void update(struct tm *timestruct)
{
    time_t timeobj = mktime(timestruct);
    struct tm * temp = localtime(&timeobj);
    *timestruct = *temp;
}

int checkflag(char *com, char flag)
{
    for(int i = 0; com[i] && com[i+1]; i++)
    {
        if(com[i] == '-' && com[i+1] == flag)
        {
            return 1;
        }
    }
    return 0;
}

char buffer[MAXLINES][LINELEN]; // stores infile

int main(int argc, char *argv[])
{
    time_t tdy = time(NULL);
    today = localtime(&tdy);

    //putd(argc);
    char *filename;
    int b = 0;
    if(argc > 1)
    {
        filename = argv[1];
    }
    else
    {
        puts("error: no filename");
        exit(1);
    }

    //putd(1);
    FILE *infile = fopen(filename, "r");

    while(fscanf(infile, "%[^\n]\n", buffer[b++]) != EOF) // read infile into buffer
    {
        if(b == MAXLINES)
        {
            puts("error: too many lines");
            exit(1);
        }
    }

    //putd(2);
    char command[LINELEN];
    int comlen;
    int detail;
    while(printf("--------------------------------------------------------------------------------\n> "), scanf("%[^\n]", command) != EOF) // get commands
    {
        getchar();

        freeprev();

        detail = checkflag(command, 'd');

        comlen = strlen(command);

        char *c1, *c2, *c3;
        c1 = strtok(command, " ");

        //int start[3] = {1900, 1, 1};
        //int end[3] = {2200, 1, 1}; // year, month, day
        struct tm start = {.tm_mday = 1, .tm_mon = 0, .tm_year = 0};
        struct tm end = {.tm_mday = 1, .tm_mon = 0, .tm_year = 300};

        if(strlen(c1)+c1 >= command+comlen) ; // no more commands, default to all

        else // get specific range, yyyy-mm-dd yyyy-mm-dd
        {
            c2 = strtok(NULL, " ");
            if(c2[0] == 'a') ; // all

            else if(sscanf(c2, "%d%*[_-]%d%*[_-]%d", &start.tm_year, &start.tm_mon, &start.tm_mday) != 3) // default to all, probably a flag
            {
                goto defaultdate;
                //puts("error: invalid date format");
                //continue;
            }
            start.tm_year -= 1900;
            start.tm_mon--;


            if(strlen(c2) + c2 >= command+comlen) // no second date, default to all
            {
                goto defaultdate;
                //puts("error: invalid date format");
                //continue;
            }
            else
            {

                c3 = strtok(NULL, " ");
                if(c3[0] == 'a') ; // all

                else if(sscanf(c3, "%d%*[_-]%d%*[_-]%d", &end.tm_year, &end.tm_mon, &end.tm_mday) != 3) // default to all, probably a flag
                {
                    goto defaultdate;
                    //puts("error: invalid date format");
                    //continue;
                }
                end.tm_year -= 1900;
                end.tm_mon--;
            }
        }
defaultdate:

        freeprev(); // free the mallocs
        numinsts = numacts = 0; // reset

        enum mode mode = choosemode(c1);
        if(mode == HELP || mode == ERR)
        {
            printf("%s\n", helptext);
        }
        else if(mode == QUIT)
        {
            return 0;
        }
        else if(mode == SPEC)
        {

        }
        else
        {
            // when iterating we assume all dates are in order
            int skip = 0;
            int first = 1;
            for(int i = 0; i < b; i++)
            {
                if(buffer[i][0] == '#') // comment
                    continue;

                if(trydate(buffer[i]))
                {
                    if(datecmp(start, curdate) > 0 || datecmp(curdate, end) > 0)
                    {
                        if(datecmp(curdate, end) > 0)
                        {
                            curdate = oldcurdate;
                            goto endearly;
                        }

                        skip = 1;
                        continue;
                    }
                    else
                        skip = 0;

                    if(mode == CAL && !first) // print date, summary of date, clear
                    {
                        struct tm temp = curdate;
                        curdate = oldcurdate;
                        sumdate();
                        freeprev();
                        curdate = temp;
                    }

                    if(first)
                        first = 0;
                    else if(mode == CAL)
                    {
                        oldcurdate.tm_mday++;
                        update(&oldcurdate);
                        while(datecmp(oldcurdate, curdate) < 0)
                        {

                            void update(struct tm *timestruct);
                            printf("%04d-%02d-%02d", oldcurdate.tm_year+1900, oldcurdate.tm_mon+1, oldcurdate.tm_mday);
                            update(&oldcurdate);
                            printf("(%s)\n", wdays[oldcurdate.tm_wday]);

                            putchar('\n');

                            oldcurdate.tm_mday++;
                            update(&oldcurdate);
                        }
                    }

                }
                else if(skip)
                    continue;
                else if(tryrange(buffer[i], detail)) ;
                else if(tryinst(buffer[i])) ;
            }
endearly: ;
            if(mode == CAL)
            {
                sumdate();
                freeprev();
            }
            else
            {
                printsum();
                freeprev();
            }

        }
        //if(streq(c1, "
    }


    fclose(infile);

    //struct tm time = {.tm_year = 0, .tm_mon = 0, .tm_mday = 31};
    //time_t thing;
    //thing = mktime(&time);
    //puts(ctime(&thing));
    //time.tm_mday++;
    //thing = mktime(&time);
    //puts(ctime(&thing));
}
