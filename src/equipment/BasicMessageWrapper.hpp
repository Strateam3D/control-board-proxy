#pragma once

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/error/en.h"

#include <string>
#include <memory>
#include "Tags.hpp"
#include "IsJson.hpp"

namespace strateam{
    namespace equipment{
        namespace rj = rapidjson;

		class JsonMessageWrapper {
			std::shared_ptr<rj::Document>		documentPtr_;
			rj::StringBuffer	buffer_;

		public:// == TYPEs ==
			using SourceType = rj::Document;
		public:
			JsonMessageWrapper() : documentPtr_(std::make_shared<rj::Document>()){}

            JsonMessageWrapper( SourceType const& doc )
			: documentPtr_(std::make_shared<rj::Document>()){
                documentPtr_->CopyFrom(doc, documentPtr_->GetAllocator());
            }

			JsonMessageWrapper( const char* rawInput )
			: documentPtr_(std::make_shared<rj::Document>()){
				documentPtr_->Parse(rawInput);
			}

			JsonMessageWrapper( JsonMessageWrapper const& rhs )
			:documentPtr_(rhs.documentPtr_)
			{
			}

			static JsonMessageWrapper fromSource(SourceType const& doc) {
				return JsonMessageWrapper( doc );
			}

			static JsonMessageWrapper fromData(const char* rawInput) {
				return JsonMessageWrapper( rawInput );
			}

			SourceType const& getSource()const {
				return *documentPtr_;
			}

			SourceType& getSource(){
				return *documentPtr_;
			}

			JsonMessageWrapper& encode(){
				rj::PrettyWriter<rj::StringBuffer, rj::UTF8<>, rj::UTF8<>, rj::CrtAllocator, rj::kWriteNanAndInfFlag> writer(buffer_);
				documentPtr_->Accept(writer);
				return *this;
			}

			JsonMessageWrapper& decode(){
				if (documentPtr_->HasParseError()) {
					throw std::runtime_error( "Parse error" );
				}else {
					rj::PrettyWriter<rj::StringBuffer, rj::UTF8<>, rj::UTF8<>, rj::CrtAllocator, rj::kWriteNanAndInfFlag> writer(buffer_);
					documentPtr_->Accept(writer);
				}

				return *this;
			}

			char const* data()const {
				return buffer_.GetString();
			}

			size_t size()const {
				return buffer_.GetSize();
			}

			bool isValid()const{
				return !documentPtr_ -> HasParseError();
			}
		};

        class TextMessageWrapper {
			std::string document_;
		public:// == TYPEs ==
			using SourceType = std::string;
		public:// == RawMessage ==
            TextMessageWrapper( SourceType const& source ){
                document_ = source;
            }

			TextMessageWrapper( const char* data ){
                document_ = data;
            }
            
			static TextMessageWrapper fromSource(SourceType const& doc) {
				return TextMessageWrapper(doc);
			}

			static TextMessageWrapper fromData(const char* rawInput) {
				return TextMessageWrapper( rawInput );
			}

			SourceType const& getSource()const {
				return document_;
			}

			SourceType& getSource(){
				return document_;
			}

			TextMessageWrapper& encode() {
				return *this;
			}

			TextMessageWrapper& decode() {
				return *this;
			}

			char const* data()const{
				return document_.c_str();
			}
		};

        template<typename Tag>
		struct BasicMessageWrapper {
			using type = typename std::conditional<IsJson<Tag>::value, JsonMessageWrapper, TextMessageWrapper >::type;
		};
    }
}