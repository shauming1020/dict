#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bench.h"
#include "bloom.h"

#define DICT_FILE "cities.txt"
#define OUT_FILE_DICT "experiment/output_bloom_cities"
#define OUT_FILE_1 "experiment/output_bloom_rand"
#define OUT_FILE_2 "experiment/output_bloom_prefix"
#define OUT_FILE_3 "experiment/output_bloom_first"
#define OUT_FILE_ONE "experiment/output_bloom_oneword"
#define OUT_WO_FILE_DICT "experiment/output_wo_bloom_cities"
#define OUT_WO_FILE_1 "experiment/output_wo_bloom_rand"
#define OUT_WO_FILE_2 "experiment/output_wo_bloom_prefix"
#define OUT_WO_FILE_3 "experiment/output_wo_bloom_first"
#define OUT_WO_FILE_ONE "experiment/output_wo_bloom_oneword"

#define TEST_LEN 5000
#define WORDMAX 256
#define PREFIX_LEN 3

double tvgetf()
{
    struct timespec ts;
    double sec;

    clock_gettime(CLOCK_REALTIME, &ts);
    sec = ts.tv_nsec;
    sec /= 1e9;
    sec += ts.tv_sec;

    return sec;
}

int bench_test(const tst_node *root, char *out_file, const int max)
{
    char prefix[PREFIX_LEN + 1] = "";
    char word[WORDMAX] = "";
    char **sgl;
    FILE *fp = fopen(out_file, "w");
    FILE *dict = fopen(DICT_FILE, "r");
    int idx = 0, sidx = 0;
    double t1, t2;

    if (!fp || !dict) {
        if (fp) {
            fprintf(stderr, "error: file open failed in '%s'.\n", DICT_FILE);
            fclose(fp);
        }
        if (dict) {
            fprintf(stderr, "error: file open failed in '%s'.\n", out_file);
            fclose(dict);
        }
        return 1;
    }

    sgl = (char **) malloc(sizeof(char *) * max);
    while (fscanf(dict, "%s", word) != EOF) {
        if (strlen(word) < sizeof(prefix) - 1)
            continue;
        strncpy(prefix, word, sizeof(prefix) - 1);
        t1 = tvgetf();
        tst_search_prefix(root, prefix, sgl, &sidx, max);
        t2 = tvgetf();
        fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
        idx++;
    }

    free(sgl);
    fclose(fp);
    fclose(dict);
    return 0;
}

void gen_random(char *s, const int len)
{
    static const char alphanum[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    s[len] = 0;
}

void get_outfile_name(char *output_file,
                      int case_,
                      const char *mode,
                      const char *ch)
{
    char tail[WORDMAX] = "";
    if (strcmp(mode, "CPY") == 0) {
        strcpy(tail, "_CPY.txt");
    } else {
        strcpy(tail, "_REF.txt");
    }
    if (strcmp(ch, "wo") == 0) {
        switch (case_) {
        case 1:
            strcat(output_file, OUT_WO_FILE_DICT);
            break;
        case 2:
            strcat(output_file, OUT_WO_FILE_1);
            break;
        case 3:
            strcat(output_file, OUT_WO_FILE_2);
            break;
        case 4:
            strcat(output_file, OUT_WO_FILE_3);
            break;
        case 0:
            strcat(output_file, OUT_WO_FILE_ONE);
            break;
        }
    } else {
        switch (case_) {
        case 1:
            strcat(output_file, OUT_FILE_DICT);
            break;
        case 2:
            strcat(output_file, OUT_FILE_1);
            break;
        case 3:
            strcat(output_file, OUT_FILE_2);
            break;
        case 4:
            strcat(output_file, OUT_FILE_3);
            break;
        case 0:
            strcat(output_file, OUT_FILE_ONE);
            break;
        }
    }
    strcat(output_file, tail);
}

int check_file_exists(FILE *fp, FILE *dict, const char *output_file)
{
    if (!fp || !dict) {
        if (fp) {
            fprintf(stderr, "error: file open failed in '%s'.\n", output_file);
            fclose(fp);
        }
        if (dict) {
            fprintf(stderr, "error: file open failed in '%s'.\n", DICT_FILE);
            fclose(dict);
        }
        return 1;
    }
    return 0;
}

int prefix_search_bloom(const tst_node *root,
                        bloom_t bloom,
                        const char *tmp,
                        const char *mode)
{
    char word[WORDMAX] = "";
    char output_file[WORDMAX] = "";
    FILE *fp;
    FILE *dict = fopen(DICT_FILE, "r");
    int idx = 0, sidx = 0;
    double t1, t2;
    char *sgl[1024] = {NULL};  // LMAX = 1024
    tst_node *res = NULL;

    if (strlen(tmp) > 0) {
        /* Search one word by TEST_LEN times*/
        strcpy(word, tmp);
        get_outfile_name(output_file, 0, mode, "w");
        fp = fopen(output_file, "w");
        if (check_file_exists(fp, dict, output_file))
            return 1;
        while (idx < TEST_LEN) {
            t1 = tvgetf();
            if (bloom_test(bloom, word)) {
                res = tst_search_prefix(root, word, sgl, &sidx, 1024);
                t2 = tvgetf();
                fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
            } else {
                t2 = tvgetf();
                fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
                printf("  %s not found by bloom filter.\n", word);
            }
            idx++;
        }

    } else {
        /* Case 1: string in cities */
        double x;
        srand(time(NULL));
        get_outfile_name(output_file, 1, mode, "w");
        fp = fopen(output_file, "w");
        if (check_file_exists(fp, dict, output_file))
            return 1;
        while (fscanf(dict, "%s", word) != EOF && (idx < TEST_LEN)) {
            x = (double) rand() / (RAND_MAX + 1.0);
            if (x < 0.5)
                continue;

            t1 = tvgetf();
            if (bloom_test(bloom, word)) {
                res = tst_search_prefix(root, word, sgl, &sidx, 1024);
                t2 = tvgetf();
                fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
            } else {
                t2 = tvgetf();
                fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
                printf("  %s not found by bloom filter.\n", word);
            }
            idx++;
        }

        /* Case 2: random string */
        output_file[0] = '\0';
        get_outfile_name(output_file, 2, mode, "w");
        fp = fopen(output_file, "w");
        idx = 0;
        sidx = 0;
        sgl[0] = NULL;
        while (idx < TEST_LEN) {
            gen_random(word, rand() % WORDMAX);

            t1 = tvgetf();
            if (bloom_test(bloom, word)) {
                res = tst_search_prefix(root, word, sgl, &sidx, 1024);
                t2 = tvgetf();
                fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
            } else {
                t2 = tvgetf();
                fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
                printf("  %s not found by bloom filter.\n", word);
            }
            idx++;
        }

        /* Case 3: string with the same prefix */
        output_file[0] = '\0';
        get_outfile_name(output_file, 3, mode, "w");
        fp = fopen(output_file, "w");
        idx = 0;
        sidx = 0;
        sgl[0] = NULL;
        while (fscanf(dict, "%s", word) != EOF && (idx < TEST_LEN)) {
            x = (double) rand() / (RAND_MAX + 1.0);
            if (x < 0.5)
                continue;

            word[strlen(word) - 1] = '\0';

            t1 = tvgetf();
            if (bloom_test(bloom, word)) {
                res = tst_search_prefix(root, word, sgl, &sidx, 1024);
                t2 = tvgetf();
                fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
            } else {
                t2 = tvgetf();
                fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
                printf("  %s not found by bloom filter.\n", word);
            }
            idx++;
        }

        /* Case 4: string with the different with first char*/
        output_file[0] = '\0';
        get_outfile_name(output_file, 4, mode, "w");
        fp = fopen(output_file, "w");
        idx = 0;
        sidx = 0;
        sgl[0] = NULL;
        while (fscanf(dict, "%s", word) != EOF && (idx < TEST_LEN)) {
            x = (double) rand() / (RAND_MAX + 1.0);
            if (x < 0.5)
                continue;

            word[0] = '$';

            t1 = tvgetf();
            if (bloom_test(bloom, word)) {
                res = tst_search_prefix(root, word, sgl, &sidx, 1024);
                t2 = tvgetf();
                fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
            } else {
                t2 = tvgetf();
                fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
                printf("  %s not found by bloom filter.\n", word);
            }
            idx++;
        }
    }

    fclose(fp);
    fclose(dict);
    free(res);
    return 0;
}

int prefix_search_wo_bloom(const tst_node *root,
                           bloom_t bloom,
                           const char *tmp,
                           const char *mode)
{
    char word[WORDMAX] = "";
    char output_file[WORDMAX] = "";
    FILE *fp;
    FILE *dict = fopen(DICT_FILE, "r");
    int idx = 0, sidx = 0;
    double t1, t2;
    char *sgl[1024] = {NULL};  // LMAX = 1024
    tst_node *res = NULL;

    if (strlen(tmp) > 0) {
        /* Search one word by TEST_LEN times*/
        strcpy(word, tmp);
        output_file[0] = '\0';
        get_outfile_name(output_file, 0, mode, "wo");
        fp = fopen(output_file, "w");
        if (check_file_exists(fp, dict, output_file))
            return 1;
        while (idx < TEST_LEN) {
            t1 = tvgetf();
            res = tst_search_prefix(root, word, sgl, &sidx, 1024);
            t2 = tvgetf();

            if (!res)
                printf("  %s not found by tree.\n", word);
            fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
            printf("find %s cost %f msec.\n", word, (t2 - t1) * 1000);
        }

    } else {
        /* Case 1 : */
        double x;
        srand(time(NULL));
        get_outfile_name(output_file, 1, mode, "wo");
        fp = fopen(output_file, "w");
        while (fscanf(dict, "%s", word) != EOF && (idx < TEST_LEN)) {
            x = (double) rand() / (RAND_MAX + 1.0);
            if (x < 0.5)
                continue;

            t1 = tvgetf();
            res = tst_search_prefix(root, word, sgl, &sidx, 1024);
            t2 = tvgetf();

            if (!res)
                printf("  %s not found by tree.\n", word);
            fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
            printf("find %s cost %f msec.\n", word, (t2 - t1) * 1000);
            idx++;
        }
        /* Case 2 */
        output_file[0] = '\0';
        get_outfile_name(output_file, 2, mode, "wo");
        fp = fopen(output_file, "w");
        idx = 0;
        sidx = 0;
        sgl[0] = NULL;
        while (idx < TEST_LEN) {
            gen_random(word, rand() % WORDMAX);

            t1 = tvgetf();
            res = tst_search_prefix(root, word, sgl, &sidx, 1024);
            t2 = tvgetf();

            if (!res)
                printf("  %s not found by bloom filter.\n", word);
            fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
            printf("find %s cost %f msec.\n", word, (t2 - t1) * 1000);
            idx++;
        }

        /* Case 3 : */
        output_file[0] = '\0';
        get_outfile_name(output_file, 3, mode, "wo");
        fp = fopen(output_file, "w");
        idx = 0;
        sidx = 0;
        sgl[0] = NULL;
        while (fscanf(dict, "%s", word) != EOF && (idx < TEST_LEN)) {
            x = (double) rand() / (RAND_MAX + 1.0);
            if (x < 0.5)
                continue;

            word[strlen(word) - 1] = '\0';

            t1 = tvgetf();
            res = tst_search_prefix(root, word, sgl, &sidx, 1024);
            t2 = tvgetf();

            if (!res)
                printf("  %s not found by bloom filter.\n", word);
            fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
            printf("find %s cost %f msec.\n", word, (t2 - t1) * 1000);
            idx++;
        }

        /* Case 4 : */
        output_file[0] = '\0';
        get_outfile_name(output_file, 4, mode, "wo");
        fp = fopen(output_file, "w");
        idx = 0;
        sidx = 0;
        sgl[0] = NULL;
        while (fscanf(dict, "%s", word) != EOF && (idx < TEST_LEN)) {
            x = (double) rand() / (RAND_MAX + 1.0);
            if (x < 0.5)
                continue;

            word[0] = '$';

            t1 = tvgetf();
            res = tst_search_prefix(root, word, sgl, &sidx, 1024);
            t2 = tvgetf();

            if (!res)
                printf("  %s not found by bloom filter.\n", word);
            fprintf(fp, "%d %f msec\n", idx, (t2 - t1) * 1000);
            printf("find %s cost %f msec.\n", word, (t2 - t1) * 1000);
            idx++;
        }
    }

    fclose(fp);
    fclose(dict);
    free(res);
    return 0;
}
