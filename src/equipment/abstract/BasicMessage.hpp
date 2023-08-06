#pragma once

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"

#include <string>

#include "Tags.hpp"

namespace strateam{
    namespace equipment{
        namespace rj = rapidjson;

		class JsonMessage {
			rj::Document		document_;
			rj::StringBuffer	buffer_;

		public:// == TYPEs ==
			using SourceType = rj::Document;
		public:
            JsonMessage( SourceType const& doc ){
                setSource( doc );
            }

			void setSource(SourceType const& doc) {
				document_.CopyFrom(doc, document_.GetAllocator());
				encode();
			}

			SourceType const& getSource()const {
				return document_;
			}

			SourceType& getSource(){
				return document_;
			}

			void encode(){
				rj::PrettyWriter<rj::StringBuffer, rj::UTF8<>, rj::UTF8<>, rj::CrtAllocator, rj::kWriteNanAndInfFlag> writer(buffer_);
				document_.Accept(writer);
			}

			char const* data() {
				return buffer_.GetString();
			}

			size_t size()const {
				return buffer_.GetSize();
			}

			void fromData(const char* rawInput) {
				document_.Parse(rawInput);

				if (document_.HasParseError()) {
					throw std::runtime_error( "Parse error" );
				}else {
					rj::PrettyWriter<rj::StringBuffer, rj::UTF8<>, rj::UTF8<>, rj::CrtAllocator, rj::kWriteNanAndInfFlag> writer(buffer_);
					document_.Accept(writer);
				}
			}

			bool isValid()const{
				return !document_.HasParseError();
			}
		};

        class TextMessage {
			std::string document_;
		public:// == TYPEs ==
			using SourceType = std::string;
		public:// == RawMessage ==
            TextMessage( SourceType const& source ){
                setSource( source );
            }
            
			void setSource(SourceType const& doc) {
				document_ = doc;
			}

			SourceType const& getSource()const {
				return document_;
			}

			SourceType& getSource(){
				return document_;
			}

			void encode() {}

			char const* data()const{
				return document_.c_str();
			}
		};

        template<typename Tag>
		class BasicMessage : public std::conditional<IsJson<Tag>::value, JsonMessage, TextMessage >::type {};
    }
}