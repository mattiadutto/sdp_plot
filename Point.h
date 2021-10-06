class  Point{
    private:
        int x;
        int y;

        int number_of_points;
        float coverage;

    public:
         Point(){
             Point(-1, -1);
        }
         Point(float x, float y){
            this->x = (int)x;
            this->y = (int)y;
            this->number_of_points = 0;
            this->coverage = 0;
        }

         Point(const  Point &p){
             Point(p.x, p.y);
        }

         Point &operator=(const  Point &p){
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
            printf("%p\t%d\t%d\t%d\t%f\n",this, x, y, number_of_points, getCoverage());
        }

        void setPosition(int x, int y){
            this->x = x;
            this->y = y;
        }

        int getPosition(int width){
            //printf("%d\n", (this->y* width) + this->x);
            return (this->y * width) + this->x;
        }

        float getCoverage(){
            return this->coverage / this->number_of_points;
        }

        int getX(){
            return this->x;
        }

        int getY(){
            return this->y;
        }
};