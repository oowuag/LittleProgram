/**
* 在这里放置类的简短描述，如类的名称，类的功能等。\n
   *     类的简短描述可以用一个小黑点(英文的句号)作为结束标记也可以通过在其后放置一个空行来表示结束.
   *
   * 在这里放置类的详细描述\n
   *     这里采用的是JavaDoc的注释风格，\n
   *     1个注释块的开始标记用1个斜杆紧跟着2个星号来表示\n
   *     1个注释块的结束标记用1个星号紧跟着1个斜杆来表示\n
   *     注意，doxygen会忽略你在注释中的换行，如果你想明确地告诉它，换行，那么就用斜杆后紧跟着1个n来表示\n
   *     值得注意的是 doxygen 中不仅允许在头文件(声明部分)中注释，你还可以在定义(实现部分)文件中注释\n
   *     问题是两部分注释，最后按怎样的顺序融合为最终的文档，似乎doxygen倾向于把定义(实现部分)文件中的注释放在前面\n
   *
   * 花了一下午时间，仔细看了doxygen的用户手册，写得非常详细，但也很复杂，\n
   * 于是把其中的一个实例摘出来，作为一个简单的实例，说明如何按照doxygen的规范为代码写注释\n
   * 建议把这份按照doxygen的规范注释过的代码用doxygen生成文档，看看效果
   *
   * 下面是 原始的 Blog 地址： \n 
   * http://hi.baidu.com/xxai/blog/item/3978cf397d2a4bfb3a87ceba.html \n
   * 转载请注明出处 ^_)^
*/
class Test
{
public:

      /**
       * 下面将通过这个实例说明如何在代码后注释.
           *
       * doxygen中既可以在代码前注释(就像现在的这个注释)，也可以在代码后注释
       */
     enum TEnum {
            TVal1, /**< 枚举变量 TVal1. 

                                       事实上代码后注释与代码前注释类似，\n
                                       只不过将注释的开始标记改为1个斜杆紧跟着2个星号再跟着一个小于号\n
                                       (像个小箭头一样，指像被注释的代码^_)^)\n

                                   代码后注释也分为简短注释和详细注释，同代码前注释一样，二者也是通过英文句号或空行来分隔 */

            TVal2, /**< 枚举变量 TVal2. */
            TVal3 /**< 枚举变量 TVal3. */
          }
        *enumPtr, /**< 枚举指针. 在这里放置详细注释. */
        enumVar;   /**< 枚举变量. 在这里放置详细注释. */

       /**
        * 类Test的构造函数.
        * 这里是类Test的构造函数的详细说明。
        */
       Test();

       /**
        * 类Test的析构函数.
        * 这里是类Test的析构函数的详细说明。
        */
       ̃Test();

       /**
        * 下面的这个实例将展示如何使用一些特殊标记对函数的各部分(如参数，返回值等)进行详细注释.
               *
               * 首先， 为了与JavaDoc的风格一致，特殊标记都是一个at符号紧跟着一些字母来组成的。
               *
        * @param a 一个整形变量(看，我们就是这样对函数参数进行注释的).
        * @param s 一个字符指针(这也是对函数参数进行注释).
        * @see Test() 有时候，为了能充分了解一个函数，还要适当地参考其它的相关的函数，你可以像这样，作一个链接
        * @see ̃Test()
        * @see testMeToo()
        * @see publicVar()
        * @return 返回测试的结果(就像这样说明函数的返回值)
        */
        int testMe(int a,const char *s);

      /**
        * 下面这个实例将说明如何在doxygen中写列表(有缩进)
               *
               * 减号前的缩进将保留\n
               * 如果只用减号，则不会出现顺序标记\n
               * 如果用减号紧跟着井号，则会出现顺序标记(如 1, 2, 3 ..., a, b, c ...之类)\n
               *
               * Event list
            - mouse events

              -# mouse move event
              -# mouse click event\n

              More info about the click event.
                -# mouse double click event

            - keyboard events
              -# key down event
              -# key up event
        */
        virtual void testMeToo(char c1,char c2) = 0;

       /**
        * a public variable.
        * Details.
        */
        int publicVar;

       /**
        * a function variable.
        * Details.
        */
        int (*handler)(int a,int b);
};