#include "../01-dynarray/dynarray.h"
#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
// #include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void swap(struct entry *a, struct entry *b) {
  struct entry temp = *a;
  *a = *b;
  *b = temp;
}

void reverse_array(struct DynArray *da) {
  if (da->len < 2) {
    return;
  }
  int i = 0;
  int j = da->len - 1;
  while (i < j) {
    struct entry front = da->dt[i];
    struct entry back = da->dt[j];

    da->dt[i] = back;
    da->dt[j] = front;
    i++;
    j--;
  }
}
int bigger(struct entry *a, struct entry *b) {
  if (a->st.st_mtim.tv_sec > b->st.st_mtim.tv_sec) {
    return 1;
  }
  if (a->st.st_mtim.tv_sec < b->st.st_mtim.tv_sec) {
    return -1;
  } else {
    if (a->st.st_mtim.tv_nsec > b->st.st_mtim.tv_nsec) {
      return 1;
    } else if (a->st.st_mtim.tv_nsec < b->st.st_mtim.tv_nsec) {
      return -1;
    } else {
      int res = strcmp(a->name, b->name);
      if (res == 0) {
        return 0;
      } else if (res < 0) {
        return -1;
      } else {
        return 1;
      }
    }
  }
  return 0;
}
void insertion_sort(struct DynArray *da) {
  if (da->len == 0) {
    return;
  }
  size_t i = 0;
  while (i < da->len - 1) {
    int x = bigger(&da->dt[i], &da->dt[i + 1]);
    if (x == -1) {
      swap(&da->dt[i], &da->dt[i + 1]);
      if (i != 0) {
        i--;
      }

    } else {
      i++;
    }
  };
  return;
}

void mode_string(mode_t mode, char *str) {
  if (S_ISDIR(mode))
    str[0] = 'd';
  else if (S_ISLNK(mode))
    str[0] = 'l';
  else if (S_ISCHR(mode))
    str[0] = 'c';
  else if (S_ISBLK(mode))
    str[0] = 'b';
  else if (S_ISFIFO(mode))
    str[0] = 'p';
  else if (S_ISSOCK(mode))
    str[0] = 's';
  else
    str[0] = '-';

  str[1] = (mode & S_IRUSR) ? 'r' : '-';
  str[2] = (mode & S_IWUSR) ? 'w' : '-';
  str[3] = (mode & S_IXUSR) ? 'x' : '-';
  str[4] = (mode & S_IRGRP) ? 'r' : '-';
  str[5] = (mode & S_IWGRP) ? 'w' : '-';
  str[6] = (mode & S_IXGRP) ? 'x' : '-';
  str[7] = (mode & S_IROTH) ? 'r' : '-';
  str[8] = (mode & S_IWOTH) ? 'w' : '-';
  str[9] = (mode & S_IXOTH) ? 'x' : '-';
  str[10] = '\0';
}

void print_entry(const struct entry *e, const int is_long) {
  char modes[11];
  mode_string(e->st.st_mode, modes);
  if (is_long == 1) {
    struct passwd *pw = getpwuid(e->st.st_uid);
    struct group *gr = getgrgid(e->st.st_gid);
    const char *user = pw ? pw->pw_name : "?";
    const char *group = gr ? gr->gr_name : "?";

    char timebuf[64];

    struct tm *tm = localtime(&e->st.st_mtim.tv_sec);
    if (tm == NULL) {
      return;
    }
    strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);

    printf("%s %lu %s %s %ld %s %s\n", modes, (unsigned long)e->st.st_nlink,
           user, group, (long)e->st.st_size, timebuf, e->name);
  } else {
    printf("%s\n", e->name);
  }
}

int show_all_flag = 0;
int long_list_flag = 0;
int time_sort_flag = 0;
int reverse_sort_flag = 0;

int fill(struct DynArray *da, DIR *dir, const char *path) {
  // function used for looping over the contents of direntry and fills up a
  struct dirent *direntry;
  while ((direntry = readdir(dir)) != NULL) {
    struct entry new_entry;
    struct stat st;
    char fullpath[4096];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, direntry->d_name);
    if (lstat(fullpath, &st) < 0) {
      perror(direntry->d_name);
      return -1;
    }
    snprintf(new_entry.name, sizeof new_entry.name, "%s", direntry->d_name);
    new_entry.st = st;
    if (!show_all_flag && direntry->d_name[0] == '.')
      continue;
    else
      DynArrayAdd(da, &new_entry);
  }
  // DynArray accordingly.
  return 0;
}

int main(int argc, char *argv[]) {
  int opt;

  while ((opt = getopt(argc, argv, "ralt")) != -1) {
    switch (opt) {
    case 'a':
      show_all_flag = 1;
      break;
    case 'l':
      long_list_flag = 1;
      break;
    case 't':
      time_sort_flag = 1;
      break;
    case 'r':
      reverse_sort_flag = 1;
      break;

    default:
      fprintf(stderr, "usage: %s [-l] [path]\n", argv[0]);
      return 1;
    }
  }
  const char *path = (optind < argc) ? argv[optind] : ".";

  DIR *dir = opendir(path);
  if (!dir) {
    perror("opendir");
    return 1;
  }
  struct DynArray da;
  DynArrayInit(&da);
  int f = fill(&da, dir, path);

  if (f == -1) {
    DynArrayFree(&da);
    closedir(dir);
    return -1;
  } else {
    if (time_sort_flag == 1) {
      insertion_sort(&da);
    }
    if (reverse_sort_flag == 1) {
      reverse_array(&da);
    }
    for (size_t i = 0; i < da.len; i++) {
      print_entry(&da.dt[i], long_list_flag);
    }
    closedir(dir);
    DynArrayFree(&da);
    return 0;
  }
}
