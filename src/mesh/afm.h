#pragma once

#include <vector>

struct Point {
    int id;
    double x;
    double y;
    bool touched;

    Point(int _id, double _x, double _y) : id(_id), x(_x), y(_y), touched(false) {}
};

struct FrontNode {
    Point* point;
    FrontNode* prev;
    FrontNode* next;

    explicit FrontNode(Point* p) : point(p), prev(nullptr), next(nullptr) {}
};

struct Triangle {
    int p1, p2, p3;
    Triangle(int a, int b, int c) : p1(a), p2(b), p3(c) {}
};

class AdvancingFront {
private:
    std::vector<Point> points;
    std::vector<Triangle> triangles;

    FrontNode* head;
    int frontSize;
    int numBoundaryPoints;

    void clearFront();

    static double distance(double x0, double y0, double x1, double y1) ;
    static bool circumcentre(const Point* p1, const Point* p2, const Point* p3, double& xc, double& yc, double& r) ;
    bool is_valid_delaunay(const Point *p1, const Point *p2, const Point *candidate) const;

public:
    AdvancingFront(const std::vector<std::pair<double, double>>& allPoints, const std::vector<int>& boundaryIndices);
    ~AdvancingFront();

    FrontNode* insertAfter(FrontNode* cur, Point* newPoint);
    FrontNode* removeNode(FrontNode* node);

    void collapse();

    [[nodiscard]] int getFrontSize() const { return frontSize; }
    [[nodiscard]] const std::vector<Triangle>& getTriangles() const { return triangles; }
};