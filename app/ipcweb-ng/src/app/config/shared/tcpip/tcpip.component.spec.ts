import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { TcpipComponent } from './tcpip.component';

describe('TcpipComponent', () => {
  let component: TcpipComponent;
  let fixture: ComponentFixture<TcpipComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ TcpipComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(TcpipComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
