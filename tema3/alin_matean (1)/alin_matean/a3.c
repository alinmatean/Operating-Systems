#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#define PIPE_1 "REQ_PIPE_82856"
#define PIPE_2 "RESP_PIPE_82856"

int main()
{
    //unlink(PIPE_1);
    unlink(PIPE_2);

    int fd1 = -1;
    int fd2 = -1;

    if(mkfifo(PIPE_2, 0600) != 0){
        printf("ERROR\ncannot create the response pipe\n");
        return 1;
    }

   fd1 = open(PIPE_1, O_RDONLY);
    if(fd1 == -1){
        printf("ERROR\ncannot open the request  pipe\n");
        return 1;
    }

    fd2 = open(PIPE_2, O_WRONLY);
    if(fd2 == -1){
        printf("ERROR\ncannot open the response pipe\n");
        return 1;
    }

    char x = 7;
    write(fd2, &x, 1);
    char connect[] = "CONNECT";
    if(write(fd2, &connect, 7) != 7){
        printf("write connect error\n");
    }

    char size;
    char *contents = NULL;
    unsigned int size_map = 0;
    int shmFd;
    char *data = NULL;
    char *data_map = NULL;
    off_t map_size_file = 0;
    unsigned int offset = 0;
    unsigned int value = 0;
    char *fileName = NULL;
    int sharedFD = -1;
    unsigned int file_offset = 0;
    unsigned int file_bytes = 0;
    unsigned int section_no = 0;
    unsigned int section_offset = 0;
    unsigned int section_bytes = 0;
    unsigned int logical_offset = 0;
    unsigned int logical_bytes = 0;

    char success_length = 7;
    char error_length = 5;

    for(;;){

        if(read(fd1, &size, 1) != 1){
            printf("read size error\n");
        }
        contents = (char*) malloc((size) * sizeof(char));
        if(read(fd1, contents, size) != size){
            printf("read contents error\n");
        }
        contents[(int)size] = '\0'; 

        if(strstr(contents, "PING") != NULL){
            unsigned int size_ping = 4;
            unsigned int var = 82856;
            if(write(fd2, &size_ping, 1)!= 1){
                printf("write size ping error\n");
            }
            if(write(fd2, "PING", 4)!=4){
                printf("write PING error\n");
            }
            write(fd2, &size_ping, 1);
            if(write(fd2, "PONG", 4) != 4){
                printf("write PONG error\n");
            } 

            if(write(fd2, &var, sizeof(unsigned int))!= sizeof(unsigned int)){
                printf("write var error\n");
            }
        }

        else 
        if(strcmp("EXIT", contents) == 0){
            free(contents);
            contents = NULL;
            close(fd1);
            close(fd2);
            unlink(PIPE_1);
            unlink(PIPE_2);
            break;
        }

        else
        if(strstr(contents, "CREATE_SHM") != NULL)
        {
            if(read(fd1, &size_map, sizeof(unsigned int))!=sizeof(unsigned int)){
                printf("read size map error\n");
            }
            unsigned int create_shm_length = 10;
            shmFd = shm_open("/Yo1o8J3f", O_CREAT | O_RDWR, 0664);
            if(shmFd < 0){
                if(write(fd2, &create_shm_length, 1) != 1){
                    printf("write create shm length error\n");
                }
                if(write(fd2, "CREATE_SHM", create_shm_length) != create_shm_length){
                    printf("write create shm error\n");
                }
                if(write(fd2, &error_length, 1) != 1){
                    printf("write length error\n");
                }
                if(write(fd2, "ERROR", error_length) != error_length){
                    printf("write create shm error\n");
                }

            }
            ftruncate(shmFd, size_map);
            data = (char*)mmap(NULL, size_map, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);   
            if(data == (void*)-1){

                if(write(fd2, &create_shm_length, 1) != 1){
                    printf("write create shm length error\n");
                }
                if(write(fd2, "CREATE_SHM", create_shm_length) != create_shm_length){
                    printf("write create shm error\n");
                }
                if(write(fd2, &error_length, 1) != 1){
                    printf("write length error\n");
                }
                if(write(fd2, "ERROR", error_length) != error_length){
                    printf("write create shm error\n");
                }

            }
            else{
                if(write(fd2, &create_shm_length, 1) != 1){
                    printf("write create shm length error\n");
                }
                if(write(fd2, "CREATE_SHM", create_shm_length) != create_shm_length){
                    printf("write create shm error\n");
                }
                if(write(fd2, &success_length, 1) != 1){
                    printf("write success length error\n");
                }
                if(write(fd2, "SUCCESS", success_length) != success_length){
                    printf("write create shm success error\n");
                }
            }
        }

        else 
        if(strstr(contents, "WRITE_TO_SHM") != NULL)
        {
            char error_write_shm = 12;
            if(read(fd1, &offset, sizeof(unsigned int)) != sizeof(unsigned int)){
                printf("read offset error\n");
            }
            if(read(fd1, &value, sizeof(unsigned int)) != sizeof(unsigned int)){
                printf("read value error\n");
            }
            if(offset > 0 && offset <= size_map && offset + sizeof(value) <= size_map){
                memcpy((data+offset), &value, sizeof(value));
                
                if(write(fd2, &error_write_shm, 1) != 1){
                    printf("write to shm length error\n");
                }
                if(write(fd2, "WRITE_TO_SHM", error_write_shm) != error_write_shm){
                    printf("write to shm error\n");
                }
                if(write(fd2, &success_length, 1) != 1){
                    printf("write length error\n");
                }
                if(write(fd2, "SUCCESS", success_length) != success_length){
                    printf("write create shm error\n");
                }
            }
            else{
                if(write(fd2, &error_write_shm, 1) != 1){
                    printf("write to shm length error\n");
                }
                if(write(fd2, "WRITE_TO_SHM", error_write_shm) != error_write_shm){
                    printf("write to shm error\n");
                }
                if(write(fd2, &error_length, 1) != 1){
                    printf("write length error\n");
                }
                if(write(fd2, "ERROR", error_length) != error_length){
                    printf("write create shm error\n");
                }
            }
        }

        else
        if(strstr(contents, "MAP_FILE") != NULL)
        {
            char map_file_length = 8;
            unsigned int fileName_size = 0;
            if(read(fd1, &fileName_size, 1) != 1){
                printf("file name size error\n");
            }
            fileName = (char*) malloc((fileName_size) * sizeof(char));
            if(read(fd1, fileName, fileName_size) != fileName_size){
                printf("fileName error\n");
            }
            fileName[(int)fileName_size] = '\0';
            sharedFD = open(fileName, O_RDONLY, 0664);
            
            if(sharedFD < 0){
                if(write(fd2, &map_file_length, 1) != 1){
                    printf("write file name size error\n");
                }
                if(write(fd2, "MAP_FILE", map_file_length) != map_file_length){
                    printf("MAP FILE error\n");
                }
                if(write(fd2, &error_length, 1) != 1){
                    printf("write length error\n");
                }
                if(write(fd2, "ERROR", error_length) != error_length){
                    printf("write map file error\n");
                }
            }
            else{
                map_size_file = lseek(sharedFD, 0, SEEK_END);
                lseek(sharedFD, 0 , SEEK_SET);
                data_map = (char*)mmap(NULL, map_size_file, PROT_READ, MAP_PRIVATE, sharedFD, 0);
                if(data_map == (void*)-1){
                    if(write(fd2, &map_file_length, 1) != 1){
                        printf("write map file error\n");
                    }
                    if(write(fd2, "MAP_FILE", fileName_size) != fileName_size){
                        printf("MAP FILE error\n");
                    }
                    if(write(fd2, &error_length, 1) != 1){
                        printf("write length error\n");
                    }
                    if(write(fd2, "ERROR", error_length) != error_length){
                        printf("write map file error\n");
                    }
                }
                else{
                    if(write(fd2, &map_file_length, 1) != 1){
                        printf("write mapfile l error\n");
                    }
                    if(write(fd2, "MAP_FILE", map_file_length) != map_file_length){
                        printf("map file error\n");
                    }
                    if(write(fd2, &success_length, 1) != 1){
                        printf("succes error\n");
                    }
                    if(write(fd2, "SUCCESS",success_length) != success_length){
                        printf("error\n");
                    }
                }

            }

        }

        else
        if(strncmp("READ_FROM_FILE_OFFSET", contents, 21) == 0)
        {
            char read_file_length = 21;
            if(read(fd1, &file_offset, sizeof(unsigned int)) != sizeof(unsigned int)){
                printf("read file offset error\n");
            }
            if(read(fd1, &file_bytes, sizeof(unsigned int)) != sizeof(unsigned int)){
                printf("read file bytes offset\n");
            }
            if(file_offset + file_bytes > map_size_file || data == NULL || data_map == NULL)
            {
                write(fd2, &read_file_length, 1);
                write(fd2, "READ_FROM_FILE_OFFSET", read_file_length);
                write(fd2, &error_length, 1);
                write(fd2, "ERROR", error_length);
            }
            else{
            
            memcpy(data, data_map + file_offset, file_bytes);
            //strncpy(data, data_map + file_offset, file_bytes);
            
            if(write(fd2, &read_file_length, 1) != 1){
                printf("error\n");
            }
            if(write(fd2, "READ_FROM_FILE_OFFSET", read_file_length)!=read_file_length){
                printf("read from file offset error\n");
            }
            if(write(fd2, &success_length, 1) != 1){
                printf("success length error\n");
            }
            if(write(fd2, "SUCCESS", success_length) != success_length){
                printf("success error\n");
            }
            }

        }
        else 
        if(strstr(contents, "READ_FROM_FILE_SECTION") != NULL)
        {
            char section_length = 22;
            read(fd1, &section_no, sizeof(unsigned int));
            read(fd1, &section_offset, sizeof(unsigned int));
            read(fd1, &section_bytes, sizeof(unsigned int));

            int sect_size = 0;
            int sect_offset = 0;
            int no_of_sections = 0;
            int header_size = 0;
            
            memcpy(&header_size, data_map + map_size_file - 6, 2);
            memcpy(&no_of_sections, data_map + map_size_file - header_size + 1, 1);
            memcpy(&sect_offset, data_map + map_size_file - header_size + 2 + ((section_no - 1) * 15) + 7, 4);
            memcpy(&sect_size, data_map + map_size_file - header_size + 2 + ((section_no - 1) * 15) + 11, 4);

            if(section_no <= no_of_sections && section_bytes + section_offset <= sect_size){
                
                memcpy(data, data_map + sect_offset + section_offset, section_bytes);
                
                if(write(fd2, &section_length, 1) != 1){
                    printf("write section length error\n");
                }
                if(write(fd2, "READ_FROM_FILE_SECTION", section_length)!=section_length){
                    printf("write rffs error\n");
                }
                if(write(fd2, &success_length, 1) != 1){
                    printf("success length error\n");
                }
                if(write(fd2, "SUCCESS", success_length) != success_length){
                    printf("success error\n");
                }
            }
            
            else
            {
                write(fd2, &section_length, 1);
                write(fd2, "READ_FROM_FILE_SECTION", section_length);
                write(fd2, &error_length, 1);
                write(fd2, "ERROR", error_length);
            }

        }
        else if(strncmp("READ_FROM_LOGICAL_SPACE_OFFSET", contents, 30) == 0)
        {
            char space_length = 30;
            read(fd1, &logical_offset, sizeof(unsigned int));
            read(fd1, &logical_bytes, sizeof(unsigned int));
            
            int sect_size[30];
            int sect_offset[30];
            
            int no_of_sections = 0;
            int header_size = 0;
            int m3072 = 0;
            int vec_logic[30];///vector pentru adresa logica a fiecarei sectiuni

            memcpy(&header_size, data_map + map_size_file - 6, 2);
            memcpy(&no_of_sections, data_map + map_size_file - header_size + 1, 1);
            //printf("no of sections: %d\n", no_of_sections);
            for(int i = 0; i < no_of_sections; i++)
            {
                memcpy(&sect_size[i], data_map + map_size_file - header_size + 2 + (i * 15) + 11, 4);
                memcpy(&sect_offset[i], data_map + map_size_file - header_size + 2 + (i * 15) + 7, 4);
                //printf("size: %d --- offset: %d\n", sect_size[i], sect_offset[i]);
            }

            for(int i = 0; i < no_of_sections; i++)
            {
                vec_logic[i] = m3072;
                int x = sect_size[i];
                while(x > 0)
                {
                    m3072 += 3072;
                    x-=3072;
                }
            }
            
            if(logical_offset > vec_logic[no_of_sections - 1] + sect_size[no_of_sections - 1]){
                    
                    write(fd2, &space_length, 1);
                    write(fd2, "READ_FROM_LOGICAL_SPACE_OFFSET", space_length);
                    write(fd2, &error_length, 1);
                    write(fd2, "ERROR", success_length);
            }
            else{
            for(int i = 0; i < no_of_sections; i++)
            {
                if(logical_offset >= vec_logic[i] && logical_offset < vec_logic[i+1])
                {
                    memcpy(data, data_map + sect_offset[i] + logical_offset - vec_logic[i], logical_bytes);
                    write(fd2, &space_length, 1);
                    write(fd2, "READ_FROM_LOGICAL_SPACE_OFFSET", space_length);
                    write(fd2, &success_length, 1);
                    write(fd2, "SUCCESS", success_length);
                }
            }
            }

            //memcpy(data, data_map + sect_offset[1] + logical_offset - vec_logic[1], logical_bytes);

        
        }
    }
    
    free(contents);
    free(fileName);

    munmap(data_map, map_size_file);
    munmap(data, size_map);
    close(fd1);
    close(fd2);
    return 0;
}