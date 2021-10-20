import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { IspComponent } from './isp.component';

describe('IspComponent', () => {
  let component: IspComponent;
  let fixture: ComponentFixture<IspComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ IspComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(IspComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
