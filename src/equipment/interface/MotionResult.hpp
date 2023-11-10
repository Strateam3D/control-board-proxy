#pragma once
namespace strateam{
    namespace equipment{
        class MotionResult{
        public:// == TYPES ==
            enum E {
                Success =0, Ok=Success,
                AlreadyInPosition,
                ParseError,
                AlreadyMoving,
                Interrupted, Stopped=Interrupted,
                Timeout,
                Failed, FAILED=Failed,
                Accepted,
                NotImplemented
            };
        public:
            MotionResult() =default;
            
            MotionResult( E e ): reason_(e){}

            bool success(){
                switch( reason_ ){
                    case Success: case AlreadyInPosition: case Accepted: return true;
                    default: case AlreadyMoving: case Interrupted: return false;
                }
            }
            
            operator bool(){  
                return success();  
            }
            
            operator E() const{  
                return reason_;  
            }
            

            explicit operator const char*(){
                return str();
            }

        const char* str() const {
            switch( reason_ ){
                case Success:            return "Success";
                case Accepted:           return "Accepted";
                case AlreadyInPosition:  return "Already In Position";
                case AlreadyMoving:      return "Already Moving";
                case Interrupted:        return "Interrupted";
                case Timeout:            return "Timeout";
                case FAILED:             return "FAILED";
                case ParseError:        return "ParseError";
                case NotImplemented:    return "NotImplemented";
                default: return "UNKNOWN";
            }
        }
        private:
            E reason_{ Success };
        };
    }
}
