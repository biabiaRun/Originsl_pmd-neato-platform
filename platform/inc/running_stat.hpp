/*
 * Copyright (c) 2019, Neato Robotics, Inc.. All Rights Reserved.
 *
 * This file may contain contributions from others.
 *
 * This software is proprietary to Neato Robotics, Inc. and its transference
 * and use is to be strictly controlled.
 * Transference of this software to another party requires that all of the
 * following conditions be met:
 * 	A)	Neato has a copy of a signed NDA agreement with the receiving
 *      party
 * 	B)	Neato Software Engineering has explicitly authorized the
 *      receiving party to have a copy of this software
 * 	C)	When the work is completed or terminated by the receiving party,
 *      all copies of this software that the receiving party holds must be
 *      returned to Neato, or destroyed.
 * The receiving party is under legal obligation to not disclose or  transfer
 * this software.
 * The receiving party may not appropriate, transform or re-use this software
 * for any purpose other than a Neato Robotics authorized purpose.
 */

/**
 * A class for calculating a running mean and standarard deviaiton for floating point values
 */
#ifndef INCLUDE_RUNNING_STAT_H_
#define INCLUDE_RUNNING_STAT_H_

#include <math.h>

class RunningStat {
 public:
  RunningStat() : m_n(0.0f), m_oldM(0.0f), m_newM(0.0f), m_oldS(0.0f), m_newS(0.0f) {}
  void Clear() { m_n = 0.0f; }
  void Push(float x) {
    m_n++;
    // See Knuth TAOCP vol 2, 3rd edition, page 232
    if (m_n == 1.0f) {
      m_oldM = m_newM = x;
      m_oldS = 0.0;
    } else {
      m_newM = m_oldM + (x - m_oldM) / m_n;
      m_newS = m_oldS + (x - m_oldM) * (x - m_newM);
      // set up for next iteration
      m_oldM = m_newM;
      m_oldS = m_newS;
    }
  }
  float NumDataValues() const { return m_n; }
  float Mean() const { return (m_n > 0.0f) ? m_newM : 0.0f; }
  float Variance() const { return ((m_n > 1.0f) ? m_newS / (m_n - 1) : 0.0f); }
  float StandardDeviation() const { return sqrt(Variance()); }

 private:
  float m_n, m_oldM, m_newM, m_oldS, m_newS;
};

#endif