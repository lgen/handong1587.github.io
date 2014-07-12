// Copyright 2014 BVLC and contributors.

#include <algorithm>
#include <cmath>
#include <cfloat>
#include <utility>
#include <vector>

#include "caffe/layer.hpp"
#include "caffe/vision_layers.hpp"
#include "caffe/util/math_functions.hpp"
#include "caffe/util/io.hpp"


namespace caffe {

using std::max;

template <typename Dtype>
void AccuracyLayer<Dtype>::SetUp(
  const vector<Blob<Dtype>*>& bottom, vector<Blob<Dtype>*>* top) {
  Layer<Dtype>::SetUp(bottom, top);
  top_k_ = this->layer_param_.accuracy_param().top_k();
  CHECK_EQ(bottom[0]->num(), bottom[1]->num())
      << "The data and label should have the same number.";
  CHECK_LE(top_k_, bottom[0]->count() / bottom[0]->num())
      << "top_k must be less than or equal to the number of classes.";
  CHECK_EQ(bottom[1]->channels(), 1);
  CHECK_EQ(bottom[1]->height(), 1);
  CHECK_EQ(bottom[1]->width(), 1);
  (*top)[0]->Reshape(1, 1, 1, 1);
}

template<typename Dtype>
static bool int_Dtype_pair_greater(std::pair<int, Dtype> a,
                            std::pair<int, Dtype> b) {
  return a.second > b.second;
}

template <typename Dtype>
Dtype AccuracyLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
    vector<Blob<Dtype>*>* top) {
  Dtype accuracy = 0;
  const Dtype* bottom_data = bottom[0]->cpu_data();
  const Dtype* bottom_label = bottom[1]->cpu_data();
  int num = bottom[0]->num();
  int dim = bottom[0]->count() / bottom[0]->num();
  vector<Dtype> maxval(top_k_+1);
  vector<int> max_id(top_k_+1);
  for (int i = 0; i < num; ++i) {
    // Top-k accuracy
    std::vector<std::pair<int, Dtype> > bottom_data_vector;
    for (int j = 0; j < dim; ++j) {
      bottom_data_vector.push_back(
          std::make_pair(j, bottom_data[i * dim + j]));
    }
    std::partial_sort(
        bottom_data_vector.begin(), bottom_data_vector.begin() + top_k_,
        bottom_data_vector.end(), int_Dtype_pair_greater<Dtype>);
    // check if true label is in top k predictions
    for (int k = 0; k < top_k_; k++)
      if (bottom_data_vector[k].first == static_cast<int>(bottom_label[i])) {
        ++accuracy;
        break;
      }
  }

  // LOG(INFO) << "Accuracy: " << accuracy;
  (*top)[0]->mutable_cpu_data()[0] = accuracy / num;

  // Accuracy layer should not be used as a loss function.
  return Dtype(0);
}

INSTANTIATE_CLASS(AccuracyLayer);

}  // namespace caffe
