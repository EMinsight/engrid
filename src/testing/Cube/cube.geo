// Gmsh project created on Mon Jun  8 07:21:57 2009

//H = 0.015;
H = 0.05;
L = 1.0;

Point (1) = {0, 0, 0, H};
Point (2) = {L, 0, 0, H};
Point (3) = {L, L, 0, H};
Point (4) = {0, L, 0, H};
Point (5) = {0, 0, L, H};
Point (6) = {L, 0, L, H};
Point (7) = {L, L, L, H};
Point (8) = {0, L, L, H};
Line(1) = {5, 6};
Line(2) = {6, 7};
Line(3) = {7, 8};
Line(4) = {8, 5};
Line(5) = {5, 1};
Line(6) = {1, 4};
Line(7) = {4, 8};
Line(8) = {4, 3};
Line(9) = {3, 2};
Line(10) = {2, 1};
Line(11) = {2, 6};
Line(12) = {7, 3};
Line Loop(13) = {12, -8, 7, -3};
Plane Surface(14) = {13};
Line Loop(15) = {9, 10, 6, 8};
Plane Surface(16) = {15};
Line Loop(17) = {7, 4, 5, 6};
Plane Surface(18) = {17};
Line Loop(19) = {4, 1, 2, 3};
Plane Surface(20) = {19};
Line Loop(21) = {1, -11, 10, -5};
Plane Surface(22) = {21};
Line Loop(23) = {9, 11, 2, 12};
Plane Surface(24) = {23};
Physical Surface(1) = {20, 14, 18, 24, 22, 16};