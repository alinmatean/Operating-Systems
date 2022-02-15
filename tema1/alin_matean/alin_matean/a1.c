#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>

typedef struct section{
    char sect_name[6];
    int sect_type;
    int sect_offset;
    int sect_size;
}Section_Header;

void listDirectory(const char* path)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    
    dir = opendir(path);

    if(dir == NULL){
        perror("Could not open directory!\n");
    }

    printf("SUCCESS\n");

    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
            printf("%s/%s\n", path, entry->d_name);
        }
    }

    closedir(dir);
}

void listDirectoryRecursive(const char* path)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);

    if(dir == NULL){
        perror("Could not open directory!\n");
    }


    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0) {
                printf("%s\n", fullPath);
                if(S_ISDIR(statbuf.st_mode)) {
                    listDirectoryRecursive(fullPath);
                }
            }
        }
    }
    closedir(dir);

}

void listDirectoryName(const char* path, const char* givenString)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);

    if(dir == NULL){
        perror("Could not open directory!\n");
    }


    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0){
                if(strlen(entry->d_name) >= strlen(givenString)){
                    if(strncmp(entry->d_name, givenString, strlen(givenString)) == 0){
                        printf("%s\n", fullPath);
                    }
                }
            }
        }
    }

    closedir(dir);

}

int transformP(const char *stringP)
{
    int octalP = 0;
    int v [9] = {4, 2, 1, 4, 2, 1, 4, 2, 1};
    for(int i = 0; i<3; i++){
        if(stringP[i] != '-'){
            octalP += v[i] * 100;
        }
    }

    for(int i = 3; i<6; i++){
        if(stringP[i] != '-'){
            octalP += v[i] * 10;
        }
    }

    for(int i = 6; i<9; i++){
        if(stringP[i] != '-'){
            octalP += v[i];
        }
    }
    
    int decimalP = 0;
    int i = 1;
    while(octalP){
        decimalP += (octalP%10) * i;
        i*=8;
        octalP /= 10;
    }

    return decimalP;
}

void listDirectoryPermissions(const char* path, const char* givenP)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);

    if(dir == NULL){
        perror("Could not open directory!\n");
    }
    
    int perm = transformP(givenP);
    //printf("%d-------", perm);
    
    //sprintf(permString, "%d", perm);
    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0){
                int aux = statbuf.st_mode & 0777;
                if(aux == perm){
                    printf("%s\n", fullPath);
                }
            }
        }
    }

    closedir(dir);
}

void parseSF(const char* path)
{
    int version = 0;
    int no_of_sections = 0;
    char* magic = "TvVe";
    int header_size = 0;

    char buf[4];
    int fd = open(path, O_RDONLY);
    if(fd == -1) {
        perror("Could not open input file\n");
        return;
    }
    ///citim magic
    lseek(fd, -4, SEEK_END);
    if(read(fd, buf, 4) == -1){
        perror("Could not read magic!\n");
        return;
    }
    buf[4] = '\0';
    if(strcmp(buf, magic) != 0){
        printf("ERROR\nwrong magic\n");
        return;
    }

    ///citim header_size
    lseek(fd, -6, SEEK_END);
    if(read(fd, &header_size, 2) == -1){
        perror("Could not read header_size!\n");
        return;
    }
    //printf("%d", header_size);

    ///citim version
    lseek(fd, -header_size, SEEK_END);
    if(read(fd, &version, 1) == -1){
        perror("Could not read version!\n");
        return;
    }

    if(version <37 || version > 92){
        printf("ERROR\nwrong version\n");
        return;
    }
    //printf("%d", version);

    ///citim no_of_sections
    if(read(fd, &no_of_sections, 1) == -1){
        perror("Could not read no_of_sections!\n");
        return;
    }

    if(no_of_sections < 5 || no_of_sections > 16){
        printf("ERROR\nwrong sect_nr\n");
        return;
    }
    //printf("%d", no_of_sections);
    Section_Header* sectiune = (Section_Header*)malloc(no_of_sections * sizeof(Section_Header));

    for(int i = 0; i < no_of_sections; i++){
        sectiune[i].sect_offset = 0;
        sectiune[i].sect_size = 0;
        sectiune[i].sect_type = 0;
    }

    for(int i = 0; i < no_of_sections; i++){
        if(read(fd, sectiune[i].sect_name, 6) == -1){
            perror("Could not read sect_name\n");
            free(sectiune);
            close(fd);
            return;
        }
        sectiune[i].sect_name[6] = '\0';
        if(read(fd, &sectiune[i].sect_type, 1) == -1){
            perror("Could not read sect_type\n");
            free(sectiune);
            close(fd);
            return;
        }
        if(read(fd, &sectiune[i].sect_offset, 4) == -1){
            perror("Could not read sect_offset\n");
            free(sectiune);
            close(fd);
            return;
        }
        if(read(fd, &sectiune[i].sect_size, 4) == -1){
            perror("Could not read sect_size\n");
            free(sectiune);
            close(fd);
            return;
        }
    }

    for(int i = 0; i < no_of_sections; i++){
        if(sectiune[i].sect_type != 91 && sectiune[i].sect_type != 26 && sectiune[i].sect_type != 16 && sectiune[i].sect_type != 79){
            printf("ERROR\nwrong sect_types\n");
            free(sectiune);
            close(fd);
            return;
        }
    }

    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d\n", no_of_sections);
    for(int i = 0; i < no_of_sections; i++){
        printf("section%d: %s %d %d\n", i+1, sectiune[i].sect_name, sectiune[i].sect_type, sectiune[i].sect_size);
    }

    free(sectiune);
    close(fd);

}

int SFvalid(const char* path)
{
    int ok = 0;

    int version = 0;
    int no_of_sections = 0;
    char* magic = "TvVe";
    int header_size = 0;

    char buf[4];
    int fd = open(path, O_RDONLY);
    if(fd == -1) {
        perror("Could not open input file\n");
        return 3;
    }
    ///citim magic
    lseek(fd, -4, SEEK_END);
    if(read(fd, buf, 4) == -1){
        perror("Could not read magic!\n");
        return 3;
    }
    buf[4] = '\0';
    if(strcmp(buf, magic) != 0){
        ok = 0;
        return 3;
    }

    ///citim header_size
    lseek(fd, -6, SEEK_END);
    if(read(fd, &header_size, 2) == -1){
        perror("Could not read header_size!\n");
        return 3;
    }
    //printf("%d", header_size);

    ///citim version
    lseek(fd, -header_size, SEEK_END);
    if(read(fd, &version, 1) == -1){
        perror("Could not read version!\n");
        return 3;
    }

    if(version <37 || version > 92){
        ok = 0;
        return 3;
    }
    //printf("%d", version);

    ///citim no_of_sections
    if(read(fd, &no_of_sections, 1) == -1){
        perror("Could not read no_of_sections!\n");
        return 3;
    }

    if(no_of_sections < 5 || no_of_sections > 16){
        ok = 0;
        return 3;
    }
    //printf("%d", no_of_sections);
    Section_Header* sectiune = (Section_Header*)malloc(no_of_sections * sizeof(Section_Header));

    for(int i = 0; i < no_of_sections; i++){
        sectiune[i].sect_offset = 0;
        sectiune[i].sect_size = 0;
        sectiune[i].sect_type = 0;
        sectiune[i].sect_name[6] = '\0';
    }

    for(int i = 0; i < no_of_sections; i++){
        if(read(fd, sectiune[i].sect_name, 6) == -1){
            perror("Could not read sect_name\n");
            free(sectiune);
            close(fd);
            return 3;
        }
        if(read(fd, &sectiune[i].sect_type, 1) == -1){
            perror("Could not read sect_type\n");
            free(sectiune);
            close(fd);
            return 3;
        }
        if(read(fd, &sectiune[i].sect_offset, 4) == -1){
            perror("Could not read sect_offset\n");
            free(sectiune);
            close(fd);
            return 3;
        }
        if(read(fd, &sectiune[i].sect_size, 4) == -1){
            perror("Could not read sect_size\n");
            free(sectiune);
            close(fd);
            return 3;
        }
    }

    int cnt = 0;
    for(int i = 0; i < no_of_sections; i++){
        if(sectiune[i].sect_type != 91 && sectiune[i].sect_type != 26 && sectiune[i].sect_type != 16 && sectiune[i].sect_type != 79){
            ok = 0;
            free(sectiune);
            close(fd);
            return 3;
        }
    }
    for(int i = 0; i<no_of_sections; i++){
        if(sectiune[i].sect_type == 79){
            cnt++;
        }
    }
    if(cnt >= 2){
        ok = 1;
    }

    free(sectiune);
    close(fd);
    return ok;
}

void findall(const char* path)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char fullPath[512];
    struct stat statbuf;

    dir = opendir(path);

    if(dir == NULL){
        perror("Could not open directory!\n");
    }

    while((entry = readdir(dir)) != NULL) {
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(fullPath, 512, "%s/%s", path, entry->d_name);
            if(lstat(fullPath, &statbuf) == 0) {
                if(S_ISDIR(statbuf.st_mode)) {
                    findall(fullPath);
                }
                else if(SFvalid(fullPath) == 1){
                    printf("%s\n", fullPath);
                }
            }
        }
    }
    closedir(dir);
}

void extractLine(const char* path, const int section, const int line)
{
    int version = 0;
    int no_of_sections = 0;
    char* magic = "TvVe";
    int header_size = 0;

    char buf[4];
    int fd = open(path, O_RDONLY);
    if(fd == -1) {
        perror("ERROR\nwrong file\n\n");
        return;
    }
    ///citim magic
    lseek(fd, -4, SEEK_END);
    if(read(fd, buf, 4) == -1){
        perror("Could not read magic!\n");
        return;
    }

    buf[4] = '\0';

    if(strcmp(buf, magic) != 0){
        printf("ERROR\nwrong magic\n");
        return;
    }

    ///citim header_size
    lseek(fd, -6, SEEK_END);
    if(read(fd, &header_size, 2) == -1){
        perror("Could not read header_size!\n");
        return;
    }
    //printf("%d", header_size);

    ///citim version
    lseek(fd, -header_size, SEEK_END);
    if(read(fd, &version, 1) == -1){
        perror("Could not read version!\n");
        return;
    }

    if(version <37 || version > 92){
        printf("ERROR\nwrong version\n");
        return;
    }
    //printf("%d", versions);

    ///citim no_of_sections
    if(read(fd, &no_of_sections, 1) == -1){
        perror("Could not read no_of_sections!\n");
        return;
    }

    if(no_of_sections < 5 || no_of_sections > 16){
        printf("ERROR\nwrong section\n");
        return;
    }
    
    char sect_name[6];
    int sect_type = 0;
    int sect_offset = 0;
    int sect_size = 0;
    int foundSect_offset = 0;
    int foundSect_size = 0;
    sect_name[6] = '\0';
    if(section == 1){
        lseek(fd, 7, SEEK_CUR);
        if(read(fd, &sect_offset, 4) == -1){
            perror("Could not read!\n");
            return;
        }
        if(read(fd, &sect_size, 4) == -1){
            perror("Could not read!\n");
            return;
        }
        foundSect_offset = sect_offset;
        foundSect_size = sect_size;
        goto label2;
    }
    for(int i = 1; i <= no_of_sections; i++){
        if(read(fd, sect_name,6) == -1){
            perror("Could not read!\n");
            return;
        }
        if(read(fd, &sect_type, 1) == -1){
            perror("Could not read!\n");
            return;
        }
        if(read(fd, &sect_offset, 4) == -1){
            perror("Could not read!\n");
            return;
        }
        if(read(fd, &sect_size, 4) == -1){
            perror("Could not read!\n");
            return;
        }
        if(i == section){
            foundSect_offset = sect_offset;
            foundSect_size = sect_size;
            goto label2;
        }
    }

    label2:
    lseek(fd, foundSect_offset, SEEK_SET);
    char c = '\0';
    int chNumber = 0;
    int lineNumber = 0;
    for(int i = 0; i<foundSect_size; i++){
        if(read(fd, &c, 1) != 1){
            perror("nu citesc");
        }
        chNumber++;
        if(c == '\n'){
            lineNumber++;
            if(line == lineNumber){
                goto label1;
            }
            chNumber = 0;
        }
    }
    label1:
    if(lineNumber < line){
        perror("ERROR\nwrong line\n");
        close(fd);
        return;
    }
    lseek(fd, -1, SEEK_CUR);
    for(int i = 0; i<chNumber; i++){
        if(read(fd, &c, 1) != 1){
            perror("nu citesc");
        }
        else{
            printf("%c", c);
            lseek(fd, -2, SEEK_CUR);
        }

    }
    close(fd);
}

int main(int argc, char **argv){

    if(argc >= 2){
        if(strcmp(argv[1], "variant") == 0){
            printf("82856\n");
        }
        if(strcmp(argv[1], "list") == 0){
            if(argv[2] != NULL){
                if(strncmp(argv[2], "path=", 5) == 0){
                    listDirectory(argv[2]+5);
                }
                else if(strcmp(argv[2], "recursive") == 0){
                    if(strncmp(argv[3], "path=", 5) == 0){
                        printf("SUCCESS\n");
                        listDirectoryRecursive(argv[3]+5);
                    }
                }
                else if(strncmp(argv[2], "name_starts_with=", 17) == 0){
                    if(strncmp(argv[3], "path=", 5) == 0){
                        printf("SUCCESS\n");
                        listDirectoryName(argv[3]+5, argv[2]+17);
                    }
                }
                else if(strncmp(argv[2], "permissions=", 12) == 0){
                    if(strncmp(argv[3], "path=", 5) == 0){
                        printf("SUCCESS\n");
                        listDirectoryPermissions(argv[3]+5, argv[2]+12);
                    }
                }

                else
                {
                    printf("ERROR\ninvalid directory path!\n");
                    return 3;
                }
            }
            else{
                printf("Missing second argument!\n");
            }
        }
        if(strcmp(argv[1], "parse") == 0){
            if(argv[2] != NULL){
                parseSF(argv[2]+5);
            }
        }
        if(strcmp(argv[1], "findall") == 0){
            if(strncmp(argv[2], "path=", 5) == 0){
                printf("SUCCESS\n");
                findall(argv[2]+5);
            }
            else{
                printf("ERROR\ninvalid directory path\n");
            }
        }
        if(strcmp(argv[1], "extract") == 0){
            if(strncmp(argv[2], "path=", 5) == 0){
                int parametru1 = 0, parametru2 = 0;
                sscanf(argv[3]+8, "%d", &parametru1);
                sscanf(argv[4]+5, "%d", &parametru2);
                printf("SUCCESS");
                extractLine(argv[2]+5, parametru1, parametru2);
            }
        }
    }
    return 0;
}