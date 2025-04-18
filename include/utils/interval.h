#pragma once
// Interval

// A class representing a closed interval [a, b].
class interval {
public:
  double min, max;

  // Constructor
  interval();
  interval(double min, double max);

  // Size of the interval
  double size() const;

  // Contains and surrounds
  bool contains(double x) const;
  bool surrounds(double x) const;
  double clamp(double x) const;

  // Two static special intervals
  static const interval empty, universe;
};
