#include <string>
#include <vector>
#include "QrCode.hpp"
using namespace qrcodegen;

//int main()
//{
//
//    // Simple operation
//    QrCode qr0 = QrCode::encodeText("Hello, world!", QrCode::Ecc::MEDIUM);
//    std::string svg = qr0.toSvgString(4);
//
//    // Manual operation
//    std::vector<QrSegment> segs =
//        QrSegment::makeSegments("3141592653589793238462643383");
//    QrCode qr1 = QrCode::encodeSegments(segs, QrCode::Ecc::HIGH, 5, 5, 2, false);
//
//    return 0;
//}