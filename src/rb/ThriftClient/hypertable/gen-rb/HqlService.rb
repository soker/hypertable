#
# Autogenerated by Thrift
#
# DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
#

require 'thrift'
require 'thrift/protocol'
require 'ClientService'
require File.dirname(__FILE__) + '/Hql_types'

        module Hypertable
          module ThriftGen
            module HqlService
              class Client < ClientService::Client 
                include Thrift::Client

                def hql_exec(command, noflush, unbuffered)
                  send_hql_exec(command, noflush, unbuffered)
                  return recv_hql_exec()
                end

                def send_hql_exec(command, noflush, unbuffered)
                  send_message('hql_exec', Hql_exec_args, :command => command, :noflush => noflush, :unbuffered => unbuffered)
                end

                def recv_hql_exec()
                  result = receive_message(Hql_exec_result)
                  return result.success unless result.success.nil?
                  raise result.e unless result.e.nil?
                  raise Thrift::ApplicationException.new(Thrift::ApplicationException::MISSING_RESULT, 'hql_exec failed: unknown result')
                end

              end

              class Processor < ClientService::Processor 
                include Thrift::Processor

                def process_hql_exec(seqid, iprot, oprot)
                  args = read_args(iprot, Hql_exec_args)
                  result = Hql_exec_result.new()
                  begin
                    result.success = @handler.hql_exec(args.command, args.noflush, args.unbuffered)
                  rescue ClientException => e
                    result.e = e
                  end
                  write_result(result, oprot, 'hql_exec', seqid)
                end

              end

              # HELPER FUNCTIONS AND STRUCTURES

              class Hql_exec_args
                include Thrift::Struct
                COMMAND = 1
                NOFLUSH = 2
                UNBUFFERED = 3

                Thrift::Struct.field_accessor self, :command, :noflush, :unbuffered
                FIELDS = {
                  COMMAND => {:type => Thrift::Types::STRING, :name => 'command'},
                  NOFLUSH => {:type => Thrift::Types::BOOL, :name => 'noflush', :default => false},
                  UNBUFFERED => {:type => Thrift::Types::BOOL, :name => 'unbuffered', :default => false}
                }
                def validate
                end

              end

              class Hql_exec_result
                include Thrift::Struct
                SUCCESS = 0
                E = 1

                Thrift::Struct.field_accessor self, :success, :e
                FIELDS = {
                  SUCCESS => {:type => Thrift::Types::STRUCT, :name => 'success', :class => HqlResult},
                  E => {:type => Thrift::Types::STRUCT, :name => 'e', :class => ClientException}
                }
                def validate
                end

              end

            end

          end
        end
