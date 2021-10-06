#include "eval/public/testing/matchers.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/string_view.h"
#include "eval/public/set_util.h"

namespace google {
namespace api {
namespace expr {
namespace runtime {

void PrintTo(const CelValue& value, std::ostream* os) {
  *os << value.DebugString();
}

namespace test {
namespace {

using testing::MatcherInterface;
using testing::MatchResultListener;

class CelValueEqualImpl : public MatcherInterface<CelValue> {
 public:
  explicit CelValueEqualImpl(const CelValue& v) : value_(v) {}

  bool MatchAndExplain(CelValue arg,
                       MatchResultListener* listener) const override {
    return CelValueEqual(arg, value_);
  }

  void DescribeTo(std::ostream* os) const override {
    *os << value_.DebugString();
  }

 private:
  const CelValue& value_;
};

// used to implement matchers for CelValues
template <typename UnderlyingType>
class CelValueMatcherImpl : public testing::MatcherInterface<const CelValue&> {
 public:
  explicit CelValueMatcherImpl(testing::Matcher<UnderlyingType> m)
      : underlying_type_matcher_(m) {}
  bool MatchAndExplain(const CelValue& v,
                       testing::MatchResultListener* listener) const override {
    UnderlyingType arg;
    return v.GetValue(&arg) && underlying_type_matcher_.Matches(arg);
  }

  void DescribeTo(std::ostream* os) const override {
    CelValue::Type type =
        static_cast<CelValue::Type>(CelValue::IndexOf<UnderlyingType>::value);
    *os << absl::StrCat("type is ", CelValue::TypeName(type), " and ");
    underlying_type_matcher_.DescribeTo(os);
  }

 private:
  const testing::Matcher<UnderlyingType> underlying_type_matcher_;
};

}  // namespace

CelValueMatcher EqualsCelValue(const CelValue& v) {
  return CelValueMatcher(new CelValueEqualImpl(v));
}

CelValueMatcher IsCelBool(testing::Matcher<bool> m) {
  return CelValueMatcher(new CelValueMatcherImpl<bool>(m));
}

CelValueMatcher IsCelInt64(testing::Matcher<int64_t> m) {
  return CelValueMatcher(new CelValueMatcherImpl<int64_t>(m));
}

CelValueMatcher IsCelUint64(testing::Matcher<uint64_t> m) {
  return CelValueMatcher(new CelValueMatcherImpl<uint64_t>(m));
}

CelValueMatcher IsCelDouble(testing::Matcher<double> m) {
  return CelValueMatcher(new CelValueMatcherImpl<double>(m));
}

CelValueMatcher IsCelString(testing::Matcher<absl::string_view> m) {
  return CelValueMatcher(new CelValueMatcherImpl<CelValue::StringHolder>(
      testing::Property(&CelValue::StringHolder::value, m)));
}

CelValueMatcher IsCelBytes(testing::Matcher<absl::string_view> m) {
  return CelValueMatcher(new CelValueMatcherImpl<CelValue::BytesHolder>(
      testing::Property(&CelValue::BytesHolder::value, m)));
}

CelValueMatcher IsCelMessage(testing::Matcher<const google::protobuf::Message*> m) {
  return CelValueMatcher(new CelValueMatcherImpl<const google::protobuf::Message*>(m));
}

CelValueMatcher IsCelDuration(testing::Matcher<absl::Duration> m) {
  return CelValueMatcher(new CelValueMatcherImpl<absl::Duration>(m));
}

CelValueMatcher IsCelTimestamp(testing::Matcher<absl::Time> m) {
  return CelValueMatcher(new CelValueMatcherImpl<absl::Time>(m));
}

CelValueMatcher IsCelError(testing::Matcher<absl::Status> m) {
  return CelValueMatcher(
      new CelValueMatcherImpl<const google::api::expr::runtime::CelError*>(
          testing::AllOf(testing::NotNull(), testing::Pointee(m))));
}

}  // namespace test
}  // namespace runtime
}  // namespace expr
}  // namespace api
}  // namespace google
