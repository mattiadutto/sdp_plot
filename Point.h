class Point{
    private:
        float x;
        float y;

        int number_of_points;
        float coverage;

    public:
        Point(){
            Point(-1, -1);
        }
        Point(float x, float y){
            this->x = x;
            this->y = y;
            this->number_of_points = 0;
            this->coverage = 0;
        }

        Point(const Point &p){
            Point(p.x, p.y);
        }

        Point &operator=(const Point &p){
            x = p.x;
            y = p.y;
            number_of_points = p.number_of_points;
            coverage = p.coverage;
            return *this;
        }

        void setCoverage(float c){
            number_of_points++;
            coverage += c;
        }

        void const toString(){
            printf("%p\t%f\t%f\t%d\t%f\n",this, x, y, number_of_points, getCoverage());
        }

        void setPosition(float x, float y){
            this->x = x;
            this->y = y;
        }

        int getPosition(int width){
            printf("%f\n", (this->y* width) + this->x);
            return (this->y * width) + this->x;
        }

        float getCoverage(){
            return this->coverage / this->number_of_points;
        }
};