#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<sys/stat.h>
#include<time.h>
#include<dirent.h>
#include"sync.h"


void apply_args(int argc, char *argv[], char *local_wtf, char *local_interface,
                char *remote_wtf, char *remote_interface);

void copy_file(const char *src, const char *dest, int *count);
void copy_directory(const char *src, const char *dest, int *count, int total, volatile float *progress);
void count_files(const char *path, int *count);
time_t get_modified_time(const char *path);
time_t get_newest_mtime(const char *path);

int load_config(const char *filename, char *local_wtf, char *local_interface,
        char *remote_wtf, char *remote_interface){
    FILE *file = fopen(filename, "r");

    if(file == NULL){
        perror(filename);
        return 1;
    }

    char line[256];
    while(fgets(line, sizeof(line), file) != NULL){
        char *equals = strchr(line, '=');
        if (equals == NULL) continue;

        *equals = '\0';
        char *key = line;
        char *value = equals +1;

        value[strcspn(value, "\r\n")] = '\0';

        if(strcmp(key, "local_wtf") == 0){
            strcpy(local_wtf, value);
        } else if(strcmp(key, "remote_wtf") == 0){
            strcpy(remote_wtf, value);
        } else if(strcmp(key, "local_interface") == 0){
            strcpy(local_interface, value);
        } else if(strcmp(key, "remote_interface") == 0){
            strcpy(remote_interface, value);
        }
    }
    if(local_wtf[0] == '\0' || local_interface[0] == '\0' || remote_wtf[0] == '\0' || remote_interface[0] == '\0'){
        printf("Error: missing keys in config.txt\n");
        fclose(file);
        return 1;
    }
    fclose(file);
    return 0;
}

void apply_args(int argc, char *argv[], char *local_wtf, char *local_interface,
                char *remote_wtf, char *remote_interface){

    for(int i = 1; i < argc; i++){
    char *equals = strchr(argv[i], '=');
    if(equals == NULL) continue;

    *equals = '\0';
    char *key   = argv[i];
    char *value = equals + 1;
        if(strcmp(key, "--local_wtf") == 0){
            strcpy(local_wtf, value);
        } else if(strcmp(key, "--remote_wtf") == 0){
            strcpy(remote_wtf, value);
        } else if(strcmp(key, "--local_interface") == 0){
            strcpy(local_interface, value);
        } else if(strcmp(key, "--remote_interface") == 0){
            strcpy(remote_interface, value);
        }
    }

}

void copy_file(const char *src, const char *dest, int *count){
    char buffer[4096];
    size_t bytes_read;

    FILE *src_file = fopen(src, "rb");
    if(src_file == NULL){
        perror(src);
        return;
    }

    FILE *dest_file = fopen(dest, "wb");

    if(dest_file == NULL){
        perror(dest);
        fclose(src_file);
        return;
    }


    while((bytes_read = fread(buffer, 1, sizeof(buffer), src_file)) > 0){
        fwrite(buffer, 1, bytes_read, dest_file);
    }
    (*count)++;
    fclose(src_file);
    fclose(dest_file);
}

void copy_directory(const char *src, const char *dest, int *count, int total, volatile float *progress){
    DIR *dir = opendir(src);
    if(dir == NULL){
        perror(src);
        return;

    }

    mkdir(dest);
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL){

        if(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }


        char src_path[1024];
        char dest_path[1024];


        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest, entry->d_name);

        struct stat entry_stat;
        stat(src_path, &entry_stat);

        if (S_ISDIR(entry_stat.st_mode)){
            copy_directory(src_path, dest_path, count, total, progress);
        } else {
            copy_file(src_path, dest_path, count);

        }
        *progress = (float)(*count) / total;
        printf("\rFiles copied: %d/%d", *count, total);
        fflush(stdout);
    }
        closedir(dir);
}

void sync_folder(const char *local, const char *remote, const char *name, SyncState *sync, volatile float *progress){
    time_t local_mod_time = get_newest_mtime(local);
    time_t remote_mod_time = get_newest_mtime(remote);

    int count = 0;

    if(local_mod_time == -1){
        printf("Error: local %s folder not found!", name);
        return;
    }

    int total = 0;
    count_files(local, &total);

    if(remote_mod_time == -1){
        printf("First run - copying %s to server\n", name);
        copy_directory(local, remote, &count, total, progress);
        printf("\n");
    } else {
        if(local_mod_time > remote_mod_time){
        printf("Local is newer -> copy %s to server\n", name);
        copy_directory(local, remote, &count, total, progress);
        printf("\n");
        } else if(remote_mod_time > local_mod_time){
            printf("Remote is newer -> No action needed!\n");
            *progress = 1.0f;
        } else {
            printf("%s is in sync!\n", name);
        }
    }
}

void count_files(const char *path, int *count){
    DIR *dir = opendir(path);

    if(dir == NULL){
        perror(path);
        return;
    }
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL){

        if(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }


        char src_path[1024];


        snprintf(src_path, sizeof(src_path), "%s/%s", path, entry->d_name);

        struct stat entry_stat;
        stat(src_path, &entry_stat);

        if (S_ISDIR(entry_stat.st_mode)){
            count_files(src_path, count);
        } else {
            (*count)++;
        }
    }
        closedir(dir);

}

time_t get_modified_time(const char *path){
    struct stat info;
    int result = stat(path, &info);
    if(result == -1){
        perror(path);
        return -1;
    }
    return info.st_mtime;
}

time_t get_newest_mtime(const char *path){
    time_t newest = 0;
    DIR *dir = opendir(path);

    if(dir == NULL){
        perror(path);
        return -1;
    }
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL){

        if(strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name, "..") == 0){
            continue;
        }


        char src_path[1024];


        snprintf(src_path, sizeof(src_path), "%s/%s", path, entry->d_name);

        struct stat entry_stat;
        stat(src_path, &entry_stat);

        if (S_ISDIR(entry_stat.st_mode)){
            time_t sub = get_newest_mtime(src_path);
            if(sub > newest) newest = sub;
        } else {
            if(entry_stat.st_mtime > newest){
                newest = entry_stat.st_mtime;
            }
        }
    }
        closedir(dir);
        return newest;


}

int sync_main(int argc, char *argv[], SyncState *sync){
    char local_wtf_dir[1024] = "";
    char local_interface_dir[1024] = "";
    char remote_wtf_dir[1024] = "";
    char remote_interface_dir[1024] = "";

    if(load_config("config.txt", local_wtf_dir, local_interface_dir,
               remote_wtf_dir, remote_interface_dir) != 0){
    return 1;
    }

    apply_args(argc, argv, local_wtf_dir, local_interface_dir,
           remote_wtf_dir, remote_interface_dir);

    strcpy(sync->local_wtf, local_wtf_dir);
    strcpy(sync->local_interface, local_interface_dir);
    strcpy(sync->remote_wtf, remote_wtf_dir);
    strcpy(sync->remote_interface, remote_interface_dir);

    clock_t start = clock();

    sync_folder(local_wtf_dir, remote_wtf_dir, "WTF", sync, &sync->wtf_progress);
    sync_folder(local_interface_dir, remote_interface_dir, "Interface", sync, &sync->interface_progress);

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Completed in %.02f\n", elapsed);
    return 0;
}
