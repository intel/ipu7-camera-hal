#include <vector>
#include <cstddef>
#include <algorithm>
#include <cmath>
#include "GraphResolutionConfiguratorInclude.h"
// Reusable helper that mimics MATLAB's element-wise behaviour for scalars and
// vectors. It supports the MATLAB operators used in the ported script:
//   - scalar/vector and vector/vector multiplication and division (.* ./)
//   - scalar-by-vector operations (broadcasting)
//   - ceil / floor / max / min working element-wise on vectors
// A scalar is represented as a MatlabVector of size 1. When two vectors are
// combined they must have the same length, exactly like MATLAB.
class MatlabVector
{
public:
    MatlabVector() = default;
    MatlabVector(double scalar) : mData(1, scalar) {}
    MatlabVector(std::initializer_list<double> values) : mData(values) {}
    explicit MatlabVector(std::vector<double> values) : mData(std::move(values)) {}

    size_t size() const { return mData.size(); }
    bool isScalar() const { return mData.size() == 1; }
    double operator[](size_t index) const { return mData[index]; }
    double& operator[](size_t index) { return mData[index]; }
    const std::vector<double>& data() const { return mData; }

    // Returns the single scalar value (MATLAB semantics when the result is 1x1).
    double toScalar() const
    {
        if (mData.size() != 1)
        {
            return 0;
        }
        return mData[0];
    }

    // Element-wise binary operators with MATLAB broadcasting between a scalar
    // and a vector.
    friend MatlabVector operator*(const MatlabVector& lhs, const MatlabVector& rhs)
    {
        return elementWise(lhs, rhs, [](double a, double b) { return a * b; });
    }

    friend MatlabVector operator/(const MatlabVector& lhs, const MatlabVector& rhs)
    {
        return elementWise(lhs, rhs, [](double a, double b) { return a / b; });
    }

    friend MatlabVector operator+(const MatlabVector& lhs, const MatlabVector& rhs)
    {
        return elementWise(lhs, rhs, [](double a, double b) { return a + b; });
    }

    friend MatlabVector operator-(const MatlabVector& lhs, const MatlabVector& rhs)
    {
        return elementWise(lhs, rhs, [](double a, double b) { return a - b; });
    }

    // MATLAB ceil / floor applied element-wise.
    MatlabVector ceil() const
    {
        return apply([](double v) { return std::ceil(v); });
    }

    MatlabVector floor() const
    {
        return apply([](double v) { return std::floor(v); });
    }

    // MATLAB max(v) / min(v): reduce a single vector to its extreme value.
    double max() const
    {
        return *std::max_element(mData.begin(), mData.end());
    }

    double min() const
    {
        return *std::min_element(mData.begin(), mData.end());
    }

private:
    template <typename UnaryOp>
    MatlabVector apply(UnaryOp op) const
    {
        std::vector<double> result(mData.size());
        for (size_t i = 0; i < mData.size(); ++i)
        {
            result[i] = op(mData[i]);
        }
        return MatlabVector(std::move(result));
    }

    template <typename BinaryOp>
    static MatlabVector elementWise(const MatlabVector& lhs, const MatlabVector& rhs, BinaryOp op)
    {
        if (lhs.isScalar() || rhs.isScalar())
        {
            const MatlabVector& vec = lhs.isScalar() ? rhs : lhs;
            double scalar = lhs.isScalar() ? lhs.mData[0] : rhs.mData[0];
            bool scalarIsLhs = lhs.isScalar();
            std::vector<double> result(vec.size());
            for (size_t i = 0; i < vec.size(); ++i)
            {
                result[i] = scalarIsLhs ? op(scalar, vec.mData[i]) : op(vec.mData[i], scalar);
            }
            return MatlabVector(std::move(result));
        }

        if (lhs.size() != rhs.size())
        {
            return MatlabVector(); // Return an empty vector to indicate an error (size mismatch).
        }

        std::vector<double> result(lhs.size());
        for (size_t i = 0; i < lhs.size(); ++i)
        {
            result[i] = op(lhs.mData[i], rhs.mData[i]);
        }
        return MatlabVector(std::move(result));
    }

    std::vector<double> mData;
};

// Port of the MATLAB script that populates the "consts" struct.
// The instance-specific struct fields referenced in MATLAB are kept as members
// here. MatlabVector is used to reproduce MATLAB's scalar/vector semantics.

class LbmcConfig
{
public:
    MatlabVector stripe_width;
    MatlabVector max_motion_grid_size;
    MatlabVector bpp_out;
    MatlabVector mu_stripe_width;
    uint32_t mu_stripe_inv_width_fraction = 0;
    MatlabVector mu_stripe_inv_width;
    MatlabVector internal_memory_width;
    mutable uint32_t mu_stripe_lookahead;
    mutable uint32_t mu_stripe_bleed_left;
    mutable uint32_t read_horz_offset;

    LbmcConfig(GraphResolutionConfiguratorKernelRole role) : mRole(role)
    {
        stripe_width = MatlabVector(41 * 64);

        MatlabVector bpp_ref_options;

        if (mRole == GraphResolutionConfiguratorKernelRole::McFeederSmall)
        {
            stripe_width = (stripe_width / 4).ceil();
        }

        if (mRole == GraphResolutionConfiguratorKernelRole::McFeederFull)
        {
            bpp_out = MatlabVector(10.0);
            bpp_ref_options = MatlabVector({ 10, 8 });
            mu_stripe_width = MatlabVector({ 64, 80 });
        }
        else if (mRole == GraphResolutionConfiguratorKernelRole::McFeederSmall)
        {
            bpp_out = MatlabVector(8);
            bpp_ref_options = MatlabVector(8);
            mu_stripe_width = MatlabVector(48);
        }

        MatlabVector twoMuStripeWidth = MatlabVector(2.0) * mu_stripe_width;
        MatlabVector internalMemBitsPerLineVec =
            bpp_ref_options * (twoMuStripeWidth * (stripe_width / twoMuStripeWidth).ceil());
        double internal_mem_bits_per_line = internalMemBitsPerLineVec.max();

        internal_memory_width =
            (MatlabVector(internal_mem_bits_per_line) / bpp_ref_options).floor();
    }

    // Port of the MATLAB reference-feeder cropping computation.
    void ConfigureReferenceFeeder(
        StaticGraphKernelSystemApi& mcFullSystemApi,
        int32_t tnr_ref_bpp,
        int32_t hist_tnr_output_width,
        StaticGraphFragmentDesc* kernelFragment, uint16_t& outputStartX) const
    {

        StaticGraphKernelSystemApiMcFull* systemApi = reinterpret_cast<StaticGraphKernelSystemApiMcFull*>
            (static_cast<int8_t*>(mcFullSystemApi.data) + GRA_ROUND_UP(sizeof(SystemApiRecordHeader), 4));

        const double max_motion_ratio = systemApi->max_motion_ratio / 100.0;
        double max_motion_vector_tnr = max_motion_ratio * hist_tnr_output_width;

        // mu_stripe_width_idx = (tnr_ref_bpp == 8) && ~strcmp(instance,'small');
        size_t mu_stripe_width_idx = ((tnr_ref_bpp == 8) && (mRole != GraphResolutionConfiguratorKernelRole::McFeederSmall)) ? 1 : 0;
        uint32_t mu_stripe_width_val = static_cast<uint32_t>(mu_stripe_width[mu_stripe_width_idx]);
        uint32_t cacheline_width = mu_stripe_width_val / 4;

        mu_stripe_lookahead = static_cast<uint32_t>(ceil(max_motion_vector_tnr / mu_stripe_width_val) + 1);
        read_horz_offset = static_cast<uint32_t>(outputStartX) % cacheline_width;

        mu_stripe_bleed_left =
            static_cast<uint32_t>(std::min(floor(static_cast<double>(outputStartX) / mu_stripe_width_val), static_cast<double>(mu_stripe_lookahead)));
        uint32_t mu_stripe_bleed_right =
            static_cast<uint32_t>(std::min(floor(static_cast<double>(hist_tnr_output_width - (outputStartX + kernelFragment->fragmentOutputWidth)) / mu_stripe_width_val), static_cast<double>(mu_stripe_lookahead)));
        uint32_t max_bleed = static_cast<uint32_t>(floor((internal_memory_width[mu_stripe_width_idx] - kernelFragment->fragmentOutputWidth) / mu_stripe_width_val));

        while (mu_stripe_bleed_left + mu_stripe_bleed_right > max_bleed)
        {
            mu_stripe_bleed_left = static_cast<uint32_t>(std::max((int)mu_stripe_bleed_left - 1, 0));
            mu_stripe_bleed_right = static_cast<uint32_t>(std::max((int)mu_stripe_bleed_right - 1, 0));
        }

        int32_t cropLeft = static_cast<int32_t>(
            outputStartX - read_horz_offset - mu_stripe_width_val * mu_stripe_bleed_left);
        int32_t cropRight = static_cast<int32_t>(std::min(
            mu_stripe_width_val * ceil(static_cast<double>(outputStartX + kernelFragment->fragmentOutputWidth) / mu_stripe_width_val) + mu_stripe_width_val * mu_stripe_bleed_right,
            static_cast<double>(hist_tnr_output_width)));

        kernelFragment->fragmentOutputWidth = (uint16_t)(cropRight - cropLeft);
        outputStartX = (uint16_t)cropLeft;
    }

private:
    GraphResolutionConfiguratorKernelRole mRole;
};