/*
 * Copyright (c) 2014, webvariants GmbH, http://www.webvariants.de
 *
 * This file is released under the terms of the MIT license. You can find the
 * complete text in the attached LICENSE file or online at:
 *
 * http://www.opensource.org/licenses/mit-license.php
 *
 * @author: Tino Rusch (tino.rusch@webvariants.de)
 */

#ifndef __TCPAPISERVERCOMPONENT__
#define __TCPAPISERVERCOMPONENT__

#include "logger/easylogging++.h"
#include "apiserver/ApiServerComponent.h"
#include "apiserver/JSONStreamCollector.h"
#include "world/BaseComponent.h"

#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define MAXEVENTS 128

namespace Susi {
    namespace Api {

        class TCPApiServerComponent : public Susi::System::BaseComponent {
        protected:

            std::string address;
            std::shared_ptr<Susi::Api::ApiServerComponent> api;
            std::shared_ptr<Susi::Events::IEventSystem> eventsystem;

            int sfd;

            std::thread runloop;

            std::map<int,Susi::Api::JSONStreamCollector> collectors;

            int createAndBind(const char *port, bool forceIPv6 = false){
                struct addrinfo hints;
                struct addrinfo *result, *rp;
                int s, sfd;

                memset (&hints, 0, sizeof (struct addrinfo));
                hints.ai_family = forceIPv6 ? AF_INET6 : AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
                hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
                hints.ai_flags = AI_PASSIVE;     /* All interfaces */

                s = getaddrinfo (NULL, port, &hints, &result);
                if (s != 0)
                {
                  fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
                  return -1;
                }

                for (rp = result; rp != NULL; rp = rp->ai_next) {
                    sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                    if (sfd == -1)continue;
                    s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
                    if (s == 0)break;
                    close (sfd);
                }

                if (rp == NULL){
                    fprintf (stderr, "Could not bind\n");
                    return -1;
                }

                freeaddrinfo (result);
                return sfd;
            }

            int makeSocketNonblocking (int sfd){
                int flags, s;
                flags = fcntl (sfd, F_GETFL, 0);
                if (flags == -1) {
                    perror ("fcntl");
                    return -1;
                }
                flags |= O_NONBLOCK;
                s = fcntl (sfd, F_SETFL, flags);
                if (s == -1) {
                    perror ("fcntl");
                    return -1;
                }
                return 0;
            }

            void run(){
                int s;
                int efd;
                struct epoll_event event;
                struct epoll_event *events;

                sfd = createAndBind (address.c_str(),true);
                if (sfd == -1)
                abort ();

                s = makeSocketNonblocking (sfd);
                if (s == -1)
                abort ();

                s = listen (sfd, SOMAXCONN);
                if (s == -1)
                {
                  perror ("listen");
                  abort ();
                }

                efd = epoll_create1 (0);
                if (efd == -1)
                {
                  perror ("epoll_create");
                  abort ();
                }

                event.data.fd = sfd;
                event.events = EPOLLIN | EPOLLET;
                s = epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event);
                if (s == -1)
                {
                  perror ("epoll_ctl");
                  abort ();
                }

                /* Buffer where events are returned */
                events = (struct epoll_event*)calloc (MAXEVENTS, sizeof event);

                /* The event loop */
                while (1)
                {
                  int n, i;

                  n = epoll_wait (efd, events, MAXEVENTS, -1);
                  for (i = 0; i < n; i++)
                {
                  if ((events[i].events & EPOLLERR) ||
                          (events[i].events & EPOLLHUP) ||
                          (!(events[i].events & EPOLLIN)))
                    {
                          /* An error has occured on this fd, or the socket is not
                             ready for reading (why were we notified then?) */
                      fprintf (stderr, "epoll error\n");
                      close (events[i].data.fd);
                      continue;
                    }

                  else if (sfd == events[i].data.fd)
                    {
                          /* We have a notification on the listening socket, which
                             means one or more incoming connections. */
                          while (1)
                            {
                              struct sockaddr in_addr;
                              socklen_t in_len;
                              int infd;
                              char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                              in_len = sizeof in_addr;
                              infd = accept (sfd, &in_addr, &in_len);
                              if (infd == -1)
                                {
                                  if ((errno == EAGAIN) ||
                                      (errno == EWOULDBLOCK))
                                    {
                                      /* We have processed all incoming
                                         connections. */
                                      break;
                                    }
                                  else
                                    {
                                      perror ("accept");
                                      break;
                                    }
                                }

                              s = getnameinfo (&in_addr, in_len,
                                               hbuf, sizeof hbuf,
                                               sbuf, sizeof sbuf,
                                               NI_NUMERICHOST | NI_NUMERICSERV);
                              if (s == 0)
                                {
                                  printf("Accepted connection on descriptor %d "
                                         "(host=%s, port=%s)\n", infd, hbuf, sbuf);
                                }

                              /* Make the incoming socket non-blocking and add it to the
                                 list of fds to monitor. */
                              s = makeSocketNonblocking (infd);
                              if (s == -1)
                                abort ();

                              event.data.fd = infd;
                              event.events = EPOLLIN | EPOLLET;
                              s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
                              if (s == -1)
                                {
                                  perror ("epoll_ctl");
                                  abort ();
                                }
                              collectors[infd] = JSONStreamCollector{[this,infd](std::string data){
                                auto doc = Susi::Util::Any::fromJSONString(data);
                                std::string res = doc.toJSONString();
                                write(infd,res.c_str(),res.size());
                              }};
                            }
                          continue;
                        }
                      else
                        {
                          /* We have data on the fd waiting to be read. Read and
                             display it. We must read whatever data is available
                             completely, as we are running in edge-triggered mode
                             and won't get a notification again for the same
                             data. */
                          int done = 0;

                          while (1)
                            {
                              ssize_t count;
                              char buf[512];

                              count = read (events[i].data.fd, buf, sizeof buf);
                              if (count == -1)
                                {
                                  /* If errno == EAGAIN, that means we have read all
                                     data. So go back to the main loop. */
                                  if (errno != EAGAIN)
                                    {
                                      perror ("read");
                                      done = 1;
                                    }
                                  break;
                                }
                              else if (count == 0)
                                {
                                  /* End of file. The remote has closed the
                                     connection. */
                                  done = 1;
                                  break;
                                }

                              /* collect the buffer*/
                              std::string chunk{buf,uint32_t(count)};
                              try{
                                collectors[events[i].data.fd].collect(chunk);                                
                              }catch(const std::exception & e){
                                LOG(ERROR) << "error while processing user data: "<<e.what();
                                done = 1;
                                break;
                              }
                            }

                          if (done)
                            {
                              printf ("Closed connection on descriptor %d\n",
                                      events[i].data.fd);
                              /* Closing the descriptor will make epoll remove it
                                 from the set of descriptors which are monitored. */
                              close (events[i].data.fd);
                              collectors.erase(events[i].data.fd);
                            }
                        }
                    }
                }

                free (events);
                close (sfd);
            }

        public:
            TCPApiServerComponent( Susi::System::ComponentManager * mgr,
                                   std::string addr ) :
                Susi::System::BaseComponent {mgr},
                 address {addr},
                 api {componentManager->getComponent<Susi::Api::ApiServerComponent>( "apiserver" )},
                 eventsystem {componentManager->getComponent<Susi::Events::IEventSystem>( "eventsystem" )}
            {}

            virtual void start() override {
                runloop = std::move(std::thread{[this](){run();}});
            }
            virtual void stop() override {
                close(sfd);
                if(runloop.joinable())runloop.join();
            }
            ~TCPApiServerComponent() {
                stop();
                LOG(INFO) <<  "stopped TCPApiServerComponent" ;
            }

        };

    }
}

#endif // __TCPAPISERVERCOMPONENT__
