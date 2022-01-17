void main()
{
    gl_Position = worldProjection * viewMatrix * vec4(vertex_position.xyz, 1.0);
}