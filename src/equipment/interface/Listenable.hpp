#pragma once
#include <unordered_set>

namespace strateam{
    namespace equipment{
        template< typename ListenerInterface >
        struct Listenable{        
        protected:  // == VARIALBLE MEMBERS ==
            std::unordered_set<ListenerInterface *> listeners_;
                    
        public:  // == SUBSCRIBE/UNSUBSCRIBE LISTENERS TO LISTEN SIGNALS ==
            void subscribe( ListenerInterface*l ){  
                listeners_.insert( l );  
            }
            
            void unsubscribe( ListenerInterface* l ){ 
                listeners_.erase( l );
            }
            
        protected:
            template<typename F, typename... Args>
            void notify(F&& func, Args&&... args){
                for( ListenerInterface* listener : listeners_ )
                    std::invoke( std::forward<F>(func), listener, std::forward<Args>(args)... );
            }
        };
        
    }
}