#pragma once

namespace strateam{
    namespace equipment{
        template< typename ListenerInterface >
        struct Listenable{        
        protected:  // == VARIALBLE MEMBERS ==
            ListenerInterface *listener_ = nullptr;
                    
        public:  // == SUBSCRIBE/UNSUBSCRIBE LISTENERS TO LISTEN SIGNALS ==
            void subscribe( ListenerInterface*l ){  
                listener_ = l;  
            }
            
            void unsubscribe(){ 
                listener_ = nullptr; 
            }
            
        protected:
            template<typename F, typename... Args>
            void notify(F&& func, Args&&... args){
                assert(listener_);
                std::invoke( std::forward<F>(func), listener_, std::forward<Args>(args)... );
            }
        };
        
    }
}