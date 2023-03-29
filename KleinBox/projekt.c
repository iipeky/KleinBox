#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define SIZE 1024
int sizecalculater(char *datei){
    FILE *fp = fopen(datei, "r");
    fseek(fp, 0L, SEEK_END);
    int size = ftell(fp);
    return size;
}

void cut_file(){
    char buff[250];   
    int max_len = sizeof(buff);    
    snprintf(buff, max_len, "cat a.txt |grep '>' | cut -c 3->a.txt");    
    system(buff);
}

void patch_file(char *passwort, char *filename, char *username , char *remoteip){
    char buff[250];   
    int max_len = sizeof(buff);    
    snprintf(buff, max_len, "sshpass -p %s ssh %s@%s patch -R %s unterschied.txt", passwort, username, remoteip, filename);    
    system(buff);     
}

int local_difference_calculater(char *passwort, char *filename, char *username , char *remoteip){
    char buff[250];   
    int max_len = sizeof(buff);    
    snprintf(buff, max_len, "cat '%s' | sshpass -p %s ssh %s@%s diff - %s >a.txt", filename, passwort, username, remoteip, filename);    
    system(buff);
    int size = sizecalculater("a.txt");        
    return size;
}

int difference_calculater(char *passwort, char *filename, char *username , char *remoteip){
    char buff[250];   
    int max_len = sizeof(buff);    
    int j = snprintf(buff, max_len, "sshpass -p %s ssh %s@%s cat '%s' | diff - %s | grep '>' | cut -c 3- > a.txt", passwort, username, remoteip, filename, filename);    
    system(buff);
    int size = sizecalculater("a.txt");        
    return size;
}

void see_difference(char *filename, char *username ,char *passwort, char *remoteip){
    char buffer[250];
    int max_lener = sizeof(buffer);
    snprintf(buffer, max_lener, "bash -c \'diff -u %s <(sshpass -p %s ssh %s@%s cat %s)>unterschied.txt\'", filename, passwort, username, remoteip, filename);
    system(buffer);
}

void see_localdifference(char *filename, char *username ,char *passwort, char *remoteip){
    char buffer[250];
    int max_lener = sizeof(buffer);
    snprintf(buffer, max_lener, "sshpass -p %s ssh %s@%s 'cat %s' | diff -u - %s >unterschied.txt", passwort, username, remoteip, filename, filename);
    system(buffer);
}

void send_file(char *filename, char *username, char *remoteip){
    char buff[250];
    int max_len = sizeof(buff);
    int j = snprintf(buff, max_len, "scp %s %s@%s:~/", filename, username, remoteip);
    system(buff);
}

int check_file_exists(char *passwort, char *filename, char *username, char *remoteip){
    char buffer[350];
    int max_len = sizeof(buffer);      
    snprintf(buffer, max_len, "sshpass -p %s ssh %s@%s test -f %s && echo 'Yes'>antwort.txt||echo 'No'>antwort.txt", passwort, username, remoteip, filename);
    system(buffer);   
    if(sizecalculater("antwort.txt") == 4){
        snprintf(buffer, max_len, "sshpass -p %s ssh %s@%s ls -s %s >antwort.txt" , passwort, username, remoteip, filename);
        system(buffer);
        FILE *fp = fopen("antwort.txt", "r");
        char ch = getc(fp);
        if(ch == '0'){
            return 1;  
        }
        return 0;
    }
    return 1;
}

void receive_file(char *filename, char *username, char *remoteip){
    char buffer[350];
    int max_len = sizeof(buffer);      
    snprintf(buffer, max_len, "scp %s@%s:~/%s ~/%s", username, remoteip, filename, filename);
    system(buffer);
}

void write_file(char *filename){   
    char buff[250];
    int max_len = sizeof(buff);
    int j = snprintf(buff, max_len, "patch %s unterschied.txt", filename);
    system(buff);
    return;
}

int main(int argc, char *argv[]){
    char *filename = argv[2];
    int exists;
    char username[SIZE];
    char ipremote[SIZE];
    char passwort[SIZE];
    int size;
    FILE * fp;
    if (argc == 2 || argv[1] == "--help"){
        perror("Nutzung: ./projekt [OPTION] <DATEINAME> [OPTION]\n Richtige Nutzung: ./projekt --file <DATEINAME> (--recover)\nDateien übertragen und herunterladen\n--file  Datei setzen, die übertragen wird\n--recover  Datei herunterladen(muss nach dem Dateiname stehen)\n--help  um diese Hilfe zu sehen ");
        exit(1);
    }
    if(access("config.txt", F_OK)== 0){
        fp = fopen("config.txt", "r"); 
        fscanf(fp, "%s %s %s",ipremote, username, passwort);    
    }else{
        printf(">>>>Geben Sie die internel IP-Addresse des computer2 ein: ");
        scanf("%s", ipremote);                   
        printf(">>>>Geben Sie den Username des computer2 ein: ");
        scanf("%s", username);    
        printf(">>>>Geben Sie Passwort der des computer2 ein: ");
        scanf("%s", passwort);
        fp = fopen("config.txt","w+");
        fprintf(fp, "%s %s %s", ipremote, username, passwort);
    }
    if(argc == 4 && strcmp(argv[3], "--recover") == 0){         
        printf(">>>>%s wird heruntergeladen...\n", filename);       
        if(access(filename, F_OK)== 0){        
           printf(">>>>%s existiert bereits auf dem lokalen Server, berechne die Differenz...\n", filename);
            size = local_difference_calculater(passwort, filename, username, ipremote);
            cut_file();
            printf(">>>>%s Differenz beträgt %d kB (Gesamtdateigröße ist %d kB).\n", filename, size, sizecalculater(filename));
            see_localdifference(filename, username, passwort, ipremote);
            write_file(filename);
        }else{
            receive_file(filename, username, ipremote);
        }
    }else{             
        printf(">>>>%s wird übertragen...\n", filename);
        exists = check_file_exists(passwort, filename, username, ipremote);
        if(exists == 0){
            printf(">>>>%s existiert bereits auf dem Zielserver, berechne die Differenz...\n", filename);
            int size = difference_calculater(passwort, filename, username, ipremote);
            printf(">>>>%s Differenz beträgt %d kB (Gesamtdateigröße ist %d kB).\n", filename, size, sizecalculater(filename));
            see_difference(filename, username, passwort, ipremote);
            send_file("unterschied.txt", username, ipremote);
            patch_file(passwort, filename, username , ipremote);
        }else{
            send_file(filename, username,ipremote);
        }
    }
    remove("a.txt");
    remove("unterschied.txt");
    remove("antwort.txt");
    printf(">>>>Fertig.\n");
    return 0;
}
