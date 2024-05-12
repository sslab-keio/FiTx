#include "core/Logs.hpp"

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>

#include "llvm/Support/raw_ostream.h"

namespace framework {
struct EndPoints EndPoint::createEndPointPair() {
  int fd[2];
  if (pipe2(fd, O_NONBLOCK) < 0) {
    return EndPoints();
  }

  return EndPoints{ReadEndPoint(fd[0]), WriteEndPoint(fd[1])};
}

ReadEndPoint::ReadEndPoint() : EndPoint() {}
ReadEndPoint::ReadEndPoint(int fd) : EndPoint(fd) {}
std::string ReadEndPoint::readLog() {
  std::string log;
  char buffer[kBufferSize];
  ssize_t size = 0;

  while ((size = read(Fd(), buffer, kBufferSize)) > 0) {
    log.append(buffer, size);
  }

  return log;
}

WriteEndPoint::WriteEndPoint() : EndPoint() {}
WriteEndPoint::WriteEndPoint(int fd) : EndPoint(fd) {}
void WriteEndPoint::write_log(const std::string& log) {
  if (write(Fd(), log.data(), log.size()) < 0)
    llvm::errs() << "cannot write to log\n";
  return;
}

LoggingClient::LoggingClient() {
  end_points_ = EndPoint::createEndPointPair();
  buffer_.reserve(ReadEndPoint::kBufferSize * 10);
}

void LoggingClient::log(const std::string& log) {
  buffer_.append(log);
}

void LoggingClient::flush() {
  WriteEndPoint& write_point = end_points_.write;
  if (write_point.valid())
    write_point.write_log(buffer_);
  else
    llvm::errs() << buffer_;
  buffer_.clear();
}

void LoggingClient::printLog() {
  if (end_points_.read.valid()) llvm::errs() << end_points_.read.readLog();
}

llvm::raw_ostream& operator<<(llvm::raw_ostream& ostream,
                              framework::LoggingClient& client) {
  if (client.end_points_.read.valid())
    ostream << client.end_points_.read.readLog();

  return ostream;
}

LoggingClient& LoggingClient::operator<<(const std::string& log) {
  this->log(log);
  return *this;
}

void LoggingServer::addClient(LoggingClient* client) {
  clients_.push_back(client);
}

void LoggingServer::printClientLogs() {
  for (auto client : clients_) {
    client->printLog();
  }
}

}  // namespace framework
