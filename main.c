#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<sys/stat.h>
#include<time.h>
#include<dirent.h>

void copy_file(const char *src, const char *dest, int *count);
void copy_directory(const char *src, const char *dest, int *count, int total);
void sync_folder(const char *local, const char *remote, const char *name);
void count_files(const char *path, int *count);
time_t get_modified_time(const char *path);

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

void copy_directory(const char *src, const char *dest, int *count, int total){
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
            copy_directory(src_path, dest_path, count, total);
        } else {
            copy_file(src_path, dest_path, count);

        }
        printf("\rFiles copied: %d/%d", *count, total);
        fflush(stdout);
    }
        closedir(dir);
}

void sync_folder(const char *local, const char *remote, const char *name){


    time_t local_mod_time = get_modified_time(local);


    time_t remote_mod_time = get_modified_time(remote);


    int count = 0;
    
    if(local_mod_time == -1){
        printf("Error: local %s folder not found!", name);
        return;
    }

    int total = 0;
    count_files(local, &total);

    if(remote_mod_time == -1){
        printf("First run - copying %s to server\n", name);
        copy_directory(local, remote, &count, total);
        printf("\n");
    } else {
        //compare to decide
        if(local_mod_time > remote_mod_time){
        printf("Local is newer -> copy %s to server\n", name);
        copy_directory(local, remote, &count, total);
        printf("\n");
        } else if(remote_mod_time > local_mod_time){
            printf("Remote is newer -> No action needed!\n");
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


int main(){

    const char* local_wtf_dir = "C:\\Program Files (x86)\\World of Warcraft\\_retail_\\WTF";
    const char* local_interface_dir = "C:\\Program Files (x86)\\World of Warcraft\\_retail_\\Interface";

    const char* remote_wtf_dir = "\\\\192.168.1.153\\vault\\WOW_ADDON_BACKUP\\WTF";
    const char* remote_interface_dir = "\\\\192.168.1.153\\vault\\WOW_ADDON_BACKUP\\Interface";
    sync_folder(local_wtf_dir, remote_wtf_dir, "WTF");
    sync_folder(local_interface_dir, remote_interface_dir, "Interface");

    return 0;
}
