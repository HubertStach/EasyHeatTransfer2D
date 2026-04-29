#include "afm.h"
#include <cmath>
#include <iostream>
#include <limits>
#include "../progress_bar.h"

AdvancingFront::AdvancingFront(const std::vector<std::pair<double, double>>& allPoints, const std::vector<int>& boundaryIndices)
    : head(nullptr), frontSize(0), numBoundaryPoints(boundaryIndices.size())
{
    for (size_t i = 0; i < allPoints.size(); ++i) {
        points.emplace_back(i, allPoints[i].first, allPoints[i].second);
    }

    if (boundaryIndices.empty()) return;

    FrontNode* current = nullptr;
    FrontNode* first = nullptr;

    for (int index : boundaryIndices) {
        points[index].touched = true;
        auto* newNode = new FrontNode(&points[index]);

        if (!head) {
            head = newNode;
            first = newNode;
            current = newNode;
        } else {
            current->next = newNode;
            newNode->prev = current;
            current = newNode;
        }
        frontSize++;
    }

    if (current && first) {
        current->next = first;
        first->prev = current;
    }
}

AdvancingFront::~AdvancingFront() {
    clearFront();
}

void AdvancingFront::clearFront() {
    if (!head) return;
    FrontNode* current = head;
    do {
        FrontNode* nextNode = current->next;
        delete current;
        current = nextNode;
    } while (current != head);
    head = nullptr;
    frontSize = 0;
}

FrontNode* AdvancingFront::insertAfter(FrontNode* cur, Point* newPoint) {
    if (!cur) return nullptr;
    auto* newNode = new FrontNode(newPoint);
    FrontNode* nextNode = cur->next;
    newNode->prev = cur;
    newNode->next = nextNode;
    cur->next = newNode;
    nextNode->prev = newNode;
    newPoint->touched = true;
    frontSize++;
    return newNode;
}

FrontNode* AdvancingFront::removeNode(FrontNode* node) {
    if (!node || frontSize == 0) return nullptr;
    if (frontSize == 1) {
        delete node;
        head = nullptr;
        frontSize = 0;
        return nullptr;
    }
    FrontNode* p = node->prev;
    FrontNode* n = node->next;
    p->next = n;
    n->prev = p;
    if (node == head) head = n;
    delete node;
    frontSize--;
    return n;
}

double AdvancingFront::distance(double x0, double y0, double x1, double y1) {
    return std::sqrt((x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1));
}

double orient2d(const Point* a, const Point* b, const Point* c) {
    return (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
}

bool AdvancingFront::circumcentre(const Point* a, const Point* b, const Point* c, double& xc, double& yc, double& r) {
    double D = 2.0 * (a->x * (b->y - c->y) + b->x * (c->y - a->y) + c->x * (a->y - b->y));

    if (std::abs(D) < 1e-9) return false;

    double aSq = a->x * a->x + a->y * a->y;
    double bSq = b->x * b->x + b->y * b->y;
    double cSq = c->x * c->x + c->y * c->y;

    xc = (aSq * (b->y - c->y) + bSq * (c->y - a->y) + cSq * (a->y - b->y)) / D;
    yc = (aSq * (c->x - b->x) + bSq * (a->x - c->x) + cSq * (b->x - a->x)) / D;

    r = distance(a->x, a->y, xc, yc);
    return true;
}

bool AdvancingFront::is_valid_delaunay(const Point* p1, const Point* p2, const Point* candidate) const {
    double xc, yc, radius;

    if (!circumcentre(p1, p2, candidate, xc, yc, radius)) {
        return false;
    }

    for (const auto& p : points) {
        if (p.id == p1->id || p.id == p2->id || p.id == candidate->id) continue;

        double distToCenter = distance(xc, yc, p.x, p.y);

        if (distToCenter < radius - 1e-5) {
            return false;
        }
    }

    return true;
}

void AdvancingFront::collapse() {
    if (!head || frontSize < 3) return;
    FrontNode* cur = head;
    int maxIterations = points.size() * 100;
    int iter = 0;

    int numInteriorPoints = points.size() - numBoundaryPoints;
    int expectedTriangles = 2 * numInteriorPoints + numBoundaryPoints - 2;

    std::cout<<"Starting Advancing Front Method for Mesh generation...\n";
    while (frontSize > 3 /*&& iter < maxIterations*/) {
        iter++;

        Point* pCur = cur->point;
        Point* pNext = cur->next->point;

        Point* bestCandidate = nullptr;
        double minEdgeDist = std::numeric_limits<double>::max();
        int candidateType = 0;

        double midX = (pCur->x + pNext->x) / 2.0;
        double midY = (pCur->y + pNext->y) / 2.0;

        auto evaluateCandidate = [&](Point* C, int type) {
            if (orient2d(pCur, pNext, C) >= -1e-5) return;
            if (!is_valid_delaunay(pCur, pNext, C)) return;

            double dist = distance(midX, midY, C->x, C->y);
            if (dist < minEdgeDist) {
                minEdgeDist = dist;
                bestCandidate = C;
                candidateType = type;
            }
        };

        evaluateCandidate(cur->prev->point, 2);
        evaluateCandidate(cur->next->next->point, 3);

        for (size_t j = numBoundaryPoints; j < points.size(); ++j) {
            if (!points[j].touched) {
                evaluateCandidate(&points[j], 1);
            }
        }

        if (bestCandidate) {
            triangles.emplace_back(pCur->id, pNext->id, bestCandidate->id);

            if (candidateType == 1) {
                insertAfter(cur, bestCandidate);
            } else if (candidateType == 2) {
                cur = removeNode(cur);
            } else if (candidateType == 3) {
                removeNode(cur->next);
            }
        } else {
            cur = cur->next;
        }

        showProgress(triangles.size(), expectedTriangles);
    }

    if (frontSize == 3) {
        Point* p1 = head->point;
        Point* p2 = head->next->point;
        Point* p3 = head->prev->point;
        triangles.emplace_back(p1->id, p2->id, p3->id);

        clearFront();
    }
    std::cout<<"\n";
}