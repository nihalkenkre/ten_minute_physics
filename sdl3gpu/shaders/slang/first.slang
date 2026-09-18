struct VSInput
{

}

struct VSOutput
{

}

struct PushConstants
{

}

[[vk::push_constant]]
PushConstants pc;

[shader("vertex")]
VSOutput vertex(VSInput in, uint draw_id: SV_DrawIndex)
{
    VSOutput out;

    return out;
}

struct FSOutput
{
    float4 color;
}

[shader("fragment")]
FSOutput fragment(VSOutput in)
{
    FSOutput out;
    out.color = float4(1, 0, 0, 1);
}