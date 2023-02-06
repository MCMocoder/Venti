/**
 * @file epollimpl.h
 * @author MCMocoder (mcmocoder@ametav.com)
 * @brief
 * @version 0.1
 * @date 2023-01-25
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <sys/epoll.h>
#include <sys/signalfd.h>

#include "epoll.h"
#include "venti/context.h"
#include "venti/event.h"

namespace mocoder::venti {

void Epoll::SubmitRead(LinuxSock& sock, Buffers& buf, Event&& ev) {
  sock.ev_->Set(LinuxSock::SockEventData::READ, &sock, buf, ev);
  if (sock.ev_->rp) {
    int len = TryRead(sock);
    if (len == -EWOULDBLOCK) {
      sock.ev_->rp = false;
    } else if (len < 0) {
      ev.status_ = ERR;
      ev.error_ = -len;
      ctx_->SubmitComplete(move(ev));
      sock.ev_->Clear();
    } else {
      ev.status_ = COMPLETED;
      ev.res_ = len;
      ctx_->SubmitComplete(move(ev));
      sock.ev_->Clear();
    }
  }
}

int Epoll::TryRead(LinuxSock& sock) { return sock.Recv(sock.ev_->bufs); }

void Epoll::SubmitWrite(LinuxSock& sock, Buffers& buf, Event&& ev) {
  sock.ev_->Set(LinuxSock::SockEventData::WRITE, &sock, buf, ev);
  if (sock.ev_->wp) {
    int len = TryWrite(sock);
    if (len == -EWOULDBLOCK) {
      sock.ev_->wp = false;
    } else if (len < 0) {
      ev.status_ = ERR;
      ev.error_ = -len;
      ctx_->SubmitComplete(move(ev));
      sock.ev_->Clear();
    } else {
      ev.status_ = COMPLETED;
      ev.res_ = len;
      ctx_->SubmitComplete(move(ev));
      sock.ev_->Clear();
    }
  }
}

int Epoll::TryWrite(LinuxSock& sock) { return sock.Send(sock.ev_->bufs); }

void Epoll::SubmitConnect(LinuxSock& sock, IPAddr& addr, Event&& ev) {
  int err = sock.Connect(&addr);
  if (err == 0) {
    ev.status_ = COMPLETED;
    ctx_->SubmitComplete(move(ev));
    sock.ev_->Clear();
  } else if (err == -EWOULDBLOCK) {
    sock.ev_->Set(LinuxSock::SockEventData::CONNECT, &sock, ev);
  } else {
    ev.status_ = ERR;
    ev.error_ = -err;
    ctx_->SubmitComplete(move(ev));
    sock.ev_->Clear();
  }
}

void Epoll::SubmitAccept(LinuxSock& sock, LinuxSock& res, Event&& ev) {
  sock.ev_->Set(LinuxSock::SockEventData::ACCEPT, &sock, ev, &res);
  if (sock.ev_->rp) {
    int err = TryAccept(sock);
    if (err == 0) {
      ev.status_ = COMPLETED;
      ctx_->SubmitComplete(move(ev));
      sock.ev_->Clear();
    } else if (err == -EWOULDBLOCK) {
      sock.ev_->rp = false;
    } else {
      ev.status_ = ERR;
      ev.error_ = -err;
      ctx_->SubmitComplete(move(ev));
      sock.ev_->Clear();
    }
  }
}

int Epoll::TryAccept(LinuxSock& sock) {
  return sock.Accept((LinuxSock*)sock.ev_->data);
}

void Epoll::SubmitRecvfrom(LinuxSock& sock, IPAddr* peer, Buffers& buf,
                           Event&& ev) {
  sock.ev_->Set(LinuxSock::SockEventData::RECVFROM, &sock, buf, ev, peer);
  if (sock.ev_->rp) {
    int len = TryRecvfrom(sock);
    if (len == -EWOULDBLOCK) {
      sock.ev_->rp = false;
    } else if (len < 0) {
      ev.status_ = ERR;
      ev.error_ = -len;
      ctx_->SubmitComplete(move(ev));
      sock.ev_->Clear();
    } else {
      ev.status_ = COMPLETED;
      ev.res_ = len;
      ctx_->SubmitComplete(move(ev));
      sock.ev_->Clear();
    }
  }
}

int Epoll::TryRecvfrom(LinuxSock& sock) {
  return sock.Recvfrom(sock.ev_->bufs, (IPAddr*)sock.ev_->data);
}

void Epoll::SubmitSendto(LinuxSock& sock, IPAddr* peer, Buffers& buf,
                         Event&& ev) {
  sock.ev_->Set(LinuxSock::SockEventData::SENDTO, &sock, buf, ev, peer);
  if (sock.ev_->wp) {
    int len = TrySendto(sock);
    if (len == -EWOULDBLOCK) {
      sock.ev_->rp = false;
    } else if (len < 0) {
      ev.status_ = ERR;
      ev.error_ = -len;
      ctx_->SubmitComplete(move(ev));
      sock.ev_->Clear();
    } else {
      ev.status_ = COMPLETED;
      ev.res_ = len;
      ctx_->SubmitComplete(move(ev));
      sock.ev_->Clear();
    }
  }
}

int Epoll::TrySendto(LinuxSock& sock) {
  return sock.Sendto(sock.ev_->bufs, (IPAddr*)sock.ev_->data);
}

void Epoll::SubmitTimer(LinuxSock& sock, Event&& ev) {
  sock.ev_->Set(LinuxSock::SockEventData::TIMETICK, &sock, ev);
}

void Epoll::SubmitSignal(LinuxSock& sock, Event&& ev) {
  sock.ev_->Set(LinuxSock::SockEventData::SIGNAL, &sock, ev);
}

void Epoll::Wait() {
  epoll_event events[128];
  memset(&events, 0, sizeof(events));
  int nfds = epoll_wait(epoll_, events, 128, -1);
  for (int i = 0; i < nfds; ++i) {
    epoll_event& ev = events[i];
    LinuxSock::SockEventData& data = *(LinuxSock::SockEventData*)ev.data.ptr;
    LinuxSock& sock = *data.sock;
    if (ev.events & EPOLLERR) {
      sock.ev_->ev.status_ = ERR;
      sock.ev_->ev.error_ = sock.GetSockErr();
      ctx_->SubmitComplete(move(sock.ev_->ev));
      sock.ev_->Clear();
    } else if (ev.events & EPOLLHUP) {
      if (sock.ev_->type == LinuxSock::SockEventData::CONNECT) {
        sock.ev_->ev.status_ = ERR;
        sock.ev_->ev.error_ = 144;  // Magic Number, TODO
        ctx_->SubmitComplete(move(sock.ev_->ev));
        sock.ev_->Clear();
      }
    } else {
      if (ev.events & EPOLLIN) {
        sock.ev_->rp = true;
        switch (sock.ev_->type) {
          case LinuxSock::SockEventData::READ: {
            int len = TryRead(sock);
            if (len < 0) {
              sock.ev_->ev.status_ = ERR;
              sock.ev_->ev.error_ = -len;
              ctx_->SubmitComplete(move(sock.ev_->ev));
              sock.ev_->Clear();
            } else {
              sock.ev_->ev.status_ = COMPLETED;
              sock.ev_->ev.res_ = len;
              ctx_->SubmitComplete(move(sock.ev_->ev));
              sock.ev_->Clear();
            }
          } break;
          case LinuxSock::SockEventData::ACCEPT: {
            int err = TryAccept(sock);
            if (err == 0) {
              sock.ev_->ev.status_ = COMPLETED;
              ctx_->SubmitComplete(move(sock.ev_->ev));
              sock.ev_->Clear();
            } else {
              sock.ev_->ev.status_ = ERR;
              sock.ev_->ev.error_ = -err;
              ctx_->SubmitComplete(move(sock.ev_->ev));
              sock.ev_->Clear();
            }
          } break;
          case LinuxSock::SockEventData::RECVFROM: {
            int len = TryRecvfrom(sock);
            if (len < 0) {
              sock.ev_->ev.status_ = ERR;
              sock.ev_->ev.error_ = -len;
              ctx_->SubmitComplete(move(sock.ev_->ev));
              sock.ev_->Clear();
            } else {
              sock.ev_->ev.status_ = COMPLETED;
              sock.ev_->ev.res_ = len;
              ctx_->SubmitComplete(move(sock.ev_->ev));
              sock.ev_->Clear();
            }
          } break;
          case LinuxSock::SockEventData::TIMETICK: {
            sock.ev_->ev.status_ = COMPLETED;
            sock.ev_->ev.res_ = 0;
            ctx_->SubmitComplete(move(sock.ev_->ev));
            sock.ev_->Clear();
            sock.ev_->rp = false;
          } break;
          case LinuxSock::SockEventData::SIGNAL: {
            signalfd_siginfo info;
            Buffers bf(Buffer((char*)&info, sizeof(signalfd_siginfo)));
            sock.Recv(bf);
            sock.ev_->ev.status_ = COMPLETED;
            sock.ev_->ev.res_ = info.ssi_signo;
            ctx_->SubmitComplete(move(sock.ev_->ev));
            sock.ev_->Clear();
            sock.ev_->rp = false;
            epoll_ctl(epoll_, EPOLL_CTL_DEL, sock.sock_, nullptr);
          } break;
          case LinuxSock::SockEventData::NOOP:
          case LinuxSock::SockEventData::WRITE:
          case LinuxSock::SockEventData::SENDTO:
          case LinuxSock::SockEventData::CONNECT:
            break;
        }
      }
      if (ev.events & EPOLLOUT) {
        sock.ev_->wp = true;
        switch (sock.ev_->type) {
          case LinuxSock::SockEventData::WRITE: {
            int len = TryWrite(sock);
            if (len == -EWOULDBLOCK) {
              sock.ev_->wp = false;
            } else if (len < 0) {
              sock.ev_->ev.status_ = ERR;
              sock.ev_->ev.error_ = -len;
              ctx_->SubmitComplete(move(sock.ev_->ev));
              sock.ev_->Clear();
            } else {
              sock.ev_->ev.status_ = COMPLETED;
              sock.ev_->ev.res_ = len;
              ctx_->SubmitComplete(move(sock.ev_->ev));
              sock.ev_->Clear();
            }
          } break;
          case LinuxSock::SockEventData::CONNECT: {
            sock.ev_->ev.status_ = COMPLETED;
            ctx_->SubmitComplete(move(sock.ev_->ev));
            sock.ev_->Clear();
          } break;
          case LinuxSock::SockEventData::SENDTO: {
            int len = TrySendto(sock);
            if (len == -EWOULDBLOCK) {
              sock.ev_->rp = false;
            } else if (len < 0) {
              sock.ev_->ev.status_ = ERR;
              sock.ev_->ev.error_ = -len;
              ctx_->SubmitComplete(move(sock.ev_->ev));
              sock.ev_->Clear();
            } else {
              sock.ev_->ev.status_ = COMPLETED;
              sock.ev_->ev.res_ = len;
              ctx_->SubmitComplete(move(sock.ev_->ev));
              sock.ev_->Clear();
            }
          } break;
          case LinuxSock::SockEventData::NOOP:
          case LinuxSock::SockEventData::READ:
          case LinuxSock::SockEventData::ACCEPT:
          case LinuxSock::SockEventData::RECVFROM:
          case LinuxSock::SockEventData::TIMETICK:
          case LinuxSock::SockEventData::SIGNAL:
            break;
        }
      }
    }
  }
}

}  // namespace mocoder::venti
