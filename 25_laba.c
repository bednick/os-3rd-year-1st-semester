#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>

#define SERVER_PORT     9999
/* значение устанавливается из условий задачи                  */
#define MAX_NUM_FILES   1024
/* значение устанавливается из условий задачи                  */
#define MAX_NUM_SOC     510
/* размер буфера, который мы используем при пересылках         */
#define SIZE_BUF        1024
  /*************************************************************/
  /* Инициализируем таймаут в 1 минуты, если программа не      */
  /* Активна всё это время она завершает                       */
  /*************************************************************/
#define TIMEOUT (1 * 60 * 1000)

#define TRUE             1
#define FALSE            0

main (int argc, char *argv[])
{
    int    len, rc, on = 1;
    int    listen_sd = -1, new_sd = -1;
    int    desc_ready, end_server = FALSE, compress_array = FALSE;
    int    close_conn;
    char   buffer[SIZE_BUF];
    struct sockaddr_in   addr;
    struct pollfd fds[MAX_NUM_FILES];
    int    nfds = 0, current_size = 0, i, j;
    int    sock_n, port_n, number_n, number_this;
    struct sockaddr_in addr_n;
    struct hostent * server_n;


    if (argc < 3) 
    {
        fprintf(stderr, "usage: %s <hostname_N> <port_number_N>\n", argv[0]);
        return EXIT_FAILURE;
    }
    sock_n = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_n < 0)
    {
        perror("socket() failed");
        return EXIT_FAILURE;
    }

//
    port_n = atoi(argv[2]);
    if (port_n <= 0) {
        fprintf(stderr, "error port\n");
        return EXIT_FAILURE;
    }
    server_n = gethostbyname(argv[1]);
    if (server_n == NULL) 
    {
        fprintf(stderr, "Host not found\n");
        return EXIT_FAILURE;
    }
    bzero((char *) &addr_n, sizeof(addr_n));
    addr_n.sin_family = AF_INET;
    bcopy((char*)server_n->h_addr, (char*)&addr_n.sin_addr.s_addr, server_n->h_length);
    addr_n.sin_port = htons(port_n);
    if (connect(sock_n, (const struct sockaddr*)&addr_n, sizeof(addr_n)) < 0) 
    {
        perror("connect() N failed");
        return EXIT_FAILURE;
    }


    fprintf (stderr, "sock_n = %d\n", sock_n);


    rc = ioctl(sock_n, FIONBIO, (char *)&on);
    if (rc < 0)
    {
        perror("ioctl() sock_n failed");
        close(sock_n);
        return EXIT_FAILURE;
    }


/*************************************************************/
/* Инициализируем pollfd-структуру                           */
/*************************************************************/
    memset(fds, 0 , sizeof(fds));

/*************************************************************/
    fds[nfds].fd = sock_n;
    fds[nfds].events = POLLIN;
    number_n = nfds;
    ++nfds;

  /*************************************************************/
  /* Создание сокет AF_INET для потока входящих соединений     */
  /*************************************************************/
    listen_sd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sd < 0)
    {
        perror("socket() failed");
        close(sock_n);
        return EXIT_FAILURE;
    }

  /*************************************************************/
  /* Разрешить дескриптор сокета для повторного использования  */
  /*************************************************************/
    rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
    if (rc < 0)
    {
        perror("setsockopt() failed");
        close(sock_n);
        close(listen_sd);
        return EXIT_FAILURE;
    }

/*************************************************************/
/* Установить сокету быть блокирующими. Все гнезда для       */
/*  Входящие соединения будут также неблокирования поскольку */
/* Они наследуют это состояние от слушающего сокета.         */
/*************************************************************/
    rc = ioctl(listen_sd, FIONBIO, (char *)&on);
    if (rc < 0)
    {
        perror("ioctl() failed");
        close(sock_n);
        close(listen_sd);
        return EXIT_FAILURE;
    }

/*************************************************************/
/* Привязываем сокет                                         */
/*************************************************************/
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port        = htons(SERVER_PORT);
    rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
    if (rc < 0)
    {
        perror("bind() failed");
        close(sock_n);
        close(listen_sd);
        return EXIT_FAILURE;
    }

/*************************************************************/
/* Устанавливаем слушать нерассмотренных                     */
/*************************************************************/
    rc = listen(listen_sd, MAX_NUM_SOC);
    if (rc < 0)
    {
        perror("listen() failed");
        close(sock_n);
        close(listen_sd);
        return EXIT_FAILURE;
    }



/*************************************************************/
/* Настройка начального сокета                               */
/*************************************************************/
    fds[nfds].fd = listen_sd;
    fds[nfds].events = POLLIN;
    number_this = nfds;
    ++nfds;
/*************************************************************/
/* Цикл ждет входящих или коннекторов для поступающих данных */
/* на любой из подключенных сокетов.                         */
/*************************************************************/
    do
    {
/***********************************************************/
/* вызов poll() и ожидание TIMEOUT */
/***********************************************************/
        printf("Waiting on poll()...\n");
        rc = poll(fds, nfds, TIMEOUT);

/***********************************************************/
/* Проверка на вызов poll'а                                */
/***********************************************************/
        if (rc < 0)
        {
            perror("  poll() failed");
            break;
        }

/***********************************************************/
/* Проверяем выход по таймауту                             */
/***********************************************************/
        if (rc == 0)
        {
            printf("  poll() timed out.  End program.\n");
            break;
        }


/***********************************************************/
/* Определяем на что установленны дискрипторы              */
/***********************************************************/
        current_size = nfds;
        for (i = 0; i < current_size; i++)
        {
/*********************************************************/
/* проверяем.                          */
/*********************************************************/
            if(fds[i].revents == 0)
                continue;

/*********************************************************/
/* если это не POLLIN, то это не ожиданный результат     */
/*********************************************************/
            if(fds[i].revents != POLLIN)
            {
                printf("  Error! revents = %d\n", fds[i].revents);
                end_server = TRUE;
                break;

            }
            if (fds[i].fd == listen_sd)
            {
/*******************************************************/
/* Прослушивание дескриптор для чтения                 */
/*******************************************************/
                printf("  Listening socket is readable\n");

/*******************************************************/
/*  Принимаем все входящие соединения                  */
/*******************************************************/
                do
                {
/*****************************************************/
/* Принимать каждое входящее соединение. если        */
/* принять терпит неудачу с EWOULDBLOCK, то мы       */
/* приняли все из них. Любой другой                  */
/* Отказ от принимания заставит нас закончить        */
/*  сервер.                                          */
/*****************************************************/
                    new_sd = accept(listen_sd, NULL, NULL);


                    if (new_sd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("  accept() failed");
                            end_server = TRUE;
                        }
                        break;
                    }


                    if(ioctl(new_sd, FIONBIO, (char *)&on) < 0)
                    {
                        perror("ioctl() failed:");
                        end_server = TRUE;
                        break;
                    } else {
                        fprintf(stderr, "set nonblock %d\n", new_sd);
                    }

/*****************************************************/
/* Добавляем новое входящее соединение               */
/*****************************************************/
                    printf("  New incoming connection - %d\n", new_sd);
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    ++nfds;

/*****************************************************/
/* Резервное копирование и принятие другого          */
/* входящего подключения                             */
/*****************************************************/
                } while (new_sd != -1);
            }

/*********************************************************/
/* Это не слушающий сокет, следовательно, существующие   */
/* соединения должны быть доступны для чтения            */
/*********************************************************/

            else
            {
                printf("  Descriptor %d is readable\n", fds[i].fd);
                close_conn = FALSE;
/*******************************************************/
/* Получать все входящие данные на этом сокете         */
/* Перед тем, мы выполним цикл опроса снова            */
/*******************************************************/
                do
                {
/*****************************************************/
/* Прием данных по этому соединение до тех пор       */
/* RECV терпит неудачу с EWOULDBLOCK.                */
/* Если происходит какой-либо другой сбой,           */
/* мы будем закрывать подключение                    */
/*****************************************************/

                    rc = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                    if (rc < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("  recv() failed");
                            close_conn = TRUE;
                        }
                        break;
                    }

/*****************************************************/
/* Проверьте что соединение было закрыто клиентом    */
/*****************************************************/
                    if (rc == 0)
                    {
                        printf("  Connection closed\n");
                        close_conn = TRUE;
                        break;
                    }
/*****************************************************/
/* Данные были получены                              */
/*****************************************************/
                    len = rc;
                    printf("  %d bytes received\n", len);

/*****************************************************/
/* Отправляем на P' N                                */
/*****************************************************/
                    if (i != number_n)
                    {
                        rc = send(fds[number_n].fd, buffer, len, 0);

                        if (rc < 0)
                        {
                            perror("  send() N failed ");
                            close_conn = TRUE;
                            break;
                        }
                        else 
                        {
                            fprintf(stderr, "  send N %d", rc);
                        }
                    }
                    else
                    {
                        for (j = 0; j < nfds; j++)
                        {
                            if (j != number_n && j != number_this)
                            {
                                rc = send(fds[j].fd, buffer, len, 0);
                                if (rc < 0)
                                {
                                    perror("  send() failed ");
                                    fds[j].fd = -1;
                                    compress_array = TRUE;
                                }
                            }
                        }
                    }
                } while(TRUE);

/*******************************************************/
/* Если close_conn был выставлен но нужно закрыть      */
/* соединение(включает в себя удаление дискриптора)    */
/*******************************************************/
                if (close_conn)
                {
                    if (i == number_n)
                    {
                        fprintf(stderr, " Connection with N close\n");
                        end_server = TRUE;
                    }
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compress_array = TRUE;
                }

            }
        }/* Закончили обработку дискрипторов               */

/***********************************************************/
/* сжимаем, если нужно                                     */
/***********************************************************/
        if (compress_array)
        {
            compress_array = FALSE;
            for (i = 0; i < nfds; i++)
            {
                if (fds[i].fd == -1)
                {
                    for(j = i; j < nfds; j++)
                    {
                        fds[j].fd = fds[j+1].fd;
                    }
                    nfds--;
                }
            }
        }

    } while (end_server == FALSE); /* завершение работы сервера  */

/*************************************************************/
/* Закрываем все открытые сокеты                             */
/*************************************************************/
    for (i = 0; i < nfds; i++)
    {
        if(fds[i].fd >= 0)
        close(fds[i].fd);
    }
    return EXIT_SUCCESS;
}
