#ifndef MESSAGEHELPER_H
#define MESSAGEHELPER_H

#include "Message/GlobalMessageClient.h"

static inline void send_normal_message(const char *message){
  GlobalMessageClient::getInstance().sendString(MT_NORMAL, message); }
static inline void send_control_message(const char *message){
  GlobalMessageClient::getInstance().sendString(MT_CONTROL, message); }
static inline void send_status_message(const char *message){
  GlobalMessageClient::getInstance().sendString(MT_STATUS, message); }
static inline void send_warning_message(const char *message){
  GlobalMessageClient::getInstance().sendString(MT_WARNING, message); }
static inline void send_error_message(const char *message){
  GlobalMessageClient::getInstance().sendString(MT_ERROR, message); }
static inline void send_fatal_message(const char *message){
  GlobalMessageClient::getInstance().sendString(MT_FATAL, message); }


static inline void send_normal_message(std::string message){
  send_normal_message( message.c_str() ); }
static inline void send_control_message(std::string message){
  send_control_message( message.c_str() ); }
static inline void send_status_message(std::string message){
  send_status_message( message.c_str() ); }
static inline void send_warning_message(std::string message){
  send_warning_message( message.c_str() ); }
static inline void send_error_message(std::string message){
  send_error_message( message.c_str() ); }
static inline void send_fatal_message(std::string message){
  send_fatal_message( message.c_str() ); }

#endif
